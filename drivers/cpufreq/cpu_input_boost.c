// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

#define pr_fmt(fmt) "cpu_input_boost: " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

unsigned long last_input_time;

static __read_mostly unsigned int input_boost_freq_lp = CONFIG_INPUT_BOOST_FREQ_LP;
static __read_mostly unsigned int input_boost_freq_hp = CONFIG_INPUT_BOOST_FREQ_PERF;
static __read_mostly unsigned int input_boost_return_freq_lp = CONFIG_REMOVE_INPUT_BOOST_FREQ_LP;
static __read_mostly unsigned int input_boost_return_freq_hp = CONFIG_REMOVE_INPUT_BOOST_FREQ_PERF;
static __read_mostly unsigned int input_boost_awake_return_freq_lp = CONFIG_AWAKE_REMOVE_INPUT_BOOST_FREQ_LP;
static __read_mostly unsigned int input_boost_awake_return_freq_hp = CONFIG_AWAKE_REMOVE_INPUT_BOOST_FREQ_PERF;
static __read_mostly unsigned int general_boost_freq_lp = CONFIG_GENERAL_BOOST_FREQ_LP;
static __read_mostly unsigned int general_boost_freq_hp = CONFIG_GENERAL_BOOST_FREQ_PERF;
static __read_mostly unsigned short input_boost_duration = CONFIG_INPUT_BOOST_DURATION_MS;

module_param(input_boost_freq_lp, uint, 0644);
module_param(input_boost_freq_hp, uint, 0644);
module_param(input_boost_return_freq_lp, uint, 0644);
module_param(input_boost_return_freq_hp, uint, 0644);
module_param(input_boost_awake_return_freq_lp, uint, 0644);
module_param(input_boost_awake_return_freq_hp, uint, 0644);
module_param(general_boost_freq_lp, uint, 0644);
module_param(general_boost_freq_hp, uint, 0644);
module_param(input_boost_duration, short, 0644);

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
static __read_mostly int input_stune_boost = CONFIG_INPUT_BOOST_STUNE_LEVEL;
static __read_mostly int max_stune_boost = CONFIG_MAX_BOOST_STUNE_LEVEL;
static __read_mostly int general_stune_boost = CONFIG_GENERAL_BOOST_STUNE_LEVEL;
static __read_mostly int display_stune_boost = CONFIG_DISPLAY_BOOST_STUNE_LEVEL;

module_param_named(dynamic_stune_boost, input_stune_boost, int, 0644);
module_param(max_stune_boost, int, 0644);
module_param(general_stune_boost, int, 0644);
module_param(display_stune_boost, int, 0644);
#endif

/* Available bits for boost_drv state */
#define SCREEN_AWAKE		BIT(0)
#define INPUT_BOOST		BIT(1)
#define MAX_BOOST		BIT(2)
#define GENERAL_BOOST		BIT(3)
#define INPUT_STUNE_BOOST	BIT(4)
#define MAX_STUNE_BOOST		BIT(5)
#define GENERAL_STUNE_BOOST	BIT(6)
#define DISPLAY_STUNE_BOOST	BIT(7)

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
static bool input_stune_boost_active;
static bool max_stune_boost_active;
static bool general_stune_boost_active;
static int input_dynamic_stune_boost;
module_param(input_dynamic_stune_boost, uint, 0644);
static int general_dynamic_stune_boost;
module_param(general_dynamic_stune_boost, uint, 0644);
static int max_dynamic_stune_boost;
module_param(max_dynamic_stune_boost, uint, 0644);
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

struct boost_drv {
	struct workqueue_struct *wq;
	struct work_struct input_boost;
	struct delayed_work input_unboost;
	struct work_struct max_boost;
	struct delayed_work max_unboost;
	struct work_struct general_boost;
	struct delayed_work general_unboost;
	struct notifier_block cpu_notif;
	struct notifier_block fb_notif;
	atomic64_t max_boost_expires;
	atomic_t max_boost_dur;
	atomic64_t general_boost_expires;
	atomic_t general_boost_dur;
	atomic_t state;
	int input_stune_slot;
	int max_stune_slot;
	int general_stune_slot;
	int display_stune_slot;
};

static struct boost_drv *boost_drv_g __read_mostly;
static int boost_slot;

static u32 get_boost_freq(struct boost_drv *b, u32 cpu, u32 state)
{
	if (state & INPUT_BOOST) {
		if (cpumask_test_cpu(cpu, cpu_lp_mask))
			return input_boost_freq_lp;

		return input_boost_freq_hp;
	}

	if (cpumask_test_cpu(cpu, cpu_lp_mask))
		return general_boost_freq_lp;

	return general_boost_freq_hp;
}

static u32 get_min_freq(struct boost_drv *b, u32 cpu, u32 state)
{
	if (state & SCREEN_AWAKE) {
		if (cpumask_test_cpu(cpu, cpu_lp_mask))
			return input_boost_awake_return_freq_lp;

		return input_boost_awake_return_freq_hp;
	}

	if (cpumask_test_cpu(cpu, cpu_lp_mask))
		return input_boost_return_freq_lp;

	return input_boost_return_freq_hp;
}

static u32 get_boost_state(struct boost_drv *b)
{
	return atomic_read(&b->state);
}

static void set_boost_bit(struct boost_drv *b, u32 state)
{
	atomic_or(state, &b->state);
}

static void clear_boost_bit(struct boost_drv *b, u32 state)
{
	atomic_andnot(state, &b->state);
}

static void update_online_cpu_policy(void)
{
	u32 cpu;

	/* Only one CPU from each cluster needs to be updated */
	get_online_cpus();
	cpu = cpumask_first_and(cpu_lp_mask, cpu_online_mask);
	cpufreq_update_policy(cpu);
	cpu = cpumask_first_and(cpu_perf_mask, cpu_online_mask);
	cpufreq_update_policy(cpu);
	put_online_cpus();
}

static void set_stune_boost(struct boost_drv *b, u32 state, u32 bit, int level,
			    int *slot)
{
	if (level && !(state & bit)) {
		if (!do_stune_boost("top-app", level, slot))
			set_boost_bit(b, bit);
	}
}

static void clear_stune_boost(struct boost_drv *b, u32 state, u32 bit, int slot)
{
	if (state & bit) {
		reset_stune_boost("top-app", slot);
		clear_boost_bit(b, bit);
	}
}

static void unboost_all_cpus(struct boost_drv *b)
{
	u32 state = get_boost_state(b);

	if (!cancel_delayed_work_sync(&b->input_unboost) &&
	    !cancel_delayed_work_sync(&b->max_unboost))
		return;

	clear_boost_bit(b, INPUT_BOOST | MAX_BOOST | GENERAL_BOOST);
	update_online_cpu_policy();

	clear_stune_boost(b, state, INPUT_STUNE_BOOST, b->input_stune_slot);
	clear_stune_boost(b, state, MAX_STUNE_BOOST, b->max_stune_slot);
	clear_stune_boost(b, state, GENERAL_STUNE_BOOST, b->general_stune_slot);
}

static void __cpu_input_boost_kick(struct boost_drv *b)
{
	if (!(get_boost_state(b) & SCREEN_AWAKE))
		return;

	queue_work(b->wq, &b->input_boost);
}

void cpu_input_boost_kick(void)
{
	struct boost_drv *b = boost_drv_g;

	if (!b)
		return;

	__cpu_input_boost_kick(b);
}

static void __cpu_input_boost_kick_max(struct boost_drv *b,
				       unsigned int duration_ms)
{
	unsigned long curr_expires, new_expires;

	if (!(get_boost_state(b) & SCREEN_AWAKE))
		return;

	do {
		curr_expires = atomic64_read(&b->max_boost_expires);
		new_expires = jiffies + msecs_to_jiffies(duration_ms);

		/* Skip this boost if there's a longer boost in effect */
		if (time_after(curr_expires, new_expires))
			return;
	} while (atomic64_cmpxchg(&b->max_boost_expires, curr_expires,
		new_expires) != curr_expires);

	atomic_set(&b->max_boost_dur, duration_ms);
	queue_work(b->wq, &b->max_boost);
}

void cpu_input_boost_kick_max(unsigned int duration_ms)
{
	struct boost_drv *b = boost_drv_g;

	if (!b)
		return;

	__cpu_input_boost_kick_max(b, duration_ms);
}

static void __cpu_input_boost_kick_general(struct boost_drv *b,
	unsigned int duration_ms)
{
	unsigned long curr_expires, new_expires;

	do {
		curr_expires = atomic64_read(&b->general_boost_expires);
		new_expires = jiffies + msecs_to_jiffies(duration_ms);

		/* Skip this boost if there's a longer boost in effect */
		if (time_after(curr_expires, new_expires))
			return;
	} while (atomic64_cmpxchg(&b->general_boost_expires, curr_expires,
		new_expires) != curr_expires);

	atomic_set(&b->general_boost_dur, duration_ms);
	queue_work(b->wq, &b->general_boost);
}

void cpu_input_boost_kick_general(unsigned int duration_ms)
{
	struct boost_drv *b = boost_drv_g;
	u32 state;

	if (!b)
		return;

	state = get_boost_state(b);

	if (!(state & SCREEN_AWAKE))
		return;

	__cpu_input_boost_kick_general(b, duration_ms);
}

static void input_boost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(work, typeof(*b), input_boost);
	u32 state = get_boost_state(b);

	if (!cancel_delayed_work_sync(&b->input_unboost)) {
		set_boost_bit(b, INPUT_BOOST);
		update_online_cpu_policy();
	}

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Set input dynamic stune boost value only
	if max_dynamic_stune_boost is inactive */
	if (!max_stune_boost_active) {
		reset_stune_boost("top-app", boost_slot);
		if (input_dynamic_stune_boost > general_dynamic_stune_boost) {
			do_stune_boost("top-app", input_dynamic_stune_boost, &boost_slot);
		}
		else {
			do_stune_boost("top-app", general_dynamic_stune_boost, &boost_slot);
		}
		input_stune_boost_active = true;			
	}	
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

	queue_delayed_work(b->wq, &b->input_unboost,
			   msecs_to_jiffies(input_boost_duration));

	set_stune_boost(b, state, INPUT_STUNE_BOOST, input_stune_boost,
		&b->input_stune_slot);
}

static void input_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), input_unboost);
	u32 state = get_boost_state(b);

	clear_boost_bit(b, INPUT_BOOST);

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Reset dynamic stune boost value to the default value
	only if the boost active is input_dynamic_stune_boost */
	if (input_stune_boost_active) {
		reset_stune_boost("top-app", boost_slot);
		input_stune_boost_active = false;
	}
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

	update_online_cpu_policy();
	clear_stune_boost(b, state, INPUT_STUNE_BOOST, b->input_stune_slot);
	cpu_input_boost_kick_general(64);
}

static void max_boost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(work, typeof(*b), max_boost);
	u32 state = get_boost_state(b);

	if (!cancel_delayed_work_sync(&b->max_unboost)) {
		set_boost_bit(b, MAX_BOOST);
		update_online_cpu_policy();
	}

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Set max dynamic stune boost value */
	reset_stune_boost("top-app", boost_slot);
	if (max_dynamic_stune_boost > general_dynamic_stune_boost) {
		do_stune_boost("top-app", max_dynamic_stune_boost, &boost_slot);
	}
	else {
		do_stune_boost("top-app", general_dynamic_stune_boost, &boost_slot);
	}	
	max_stune_boost_active = true;
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */	

	queue_delayed_work(b->wq, &b->max_unboost,
		msecs_to_jiffies(atomic_read(&b->max_boost_dur)));

	set_stune_boost(b, state, MAX_STUNE_BOOST, max_stune_boost,
		&b->max_stune_slot);
}

static void max_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), max_unboost);
	u32 state = get_boost_state(b);

	clear_boost_bit(b, MAX_BOOST);

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Reset dynamic stune boost value to the default value
	only if the boost active is max_dynamic_stune_boost */
	if (max_stune_boost_active) {
		reset_stune_boost("top-app", boost_slot);
		max_stune_boost_active = false;
	}
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */
	
	update_online_cpu_policy();
	clear_stune_boost(b, state, MAX_STUNE_BOOST, b->max_stune_slot);
	cpu_input_boost_kick_general(64);
}

static void general_boost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(work, typeof(*b), general_boost);
	u32 state = get_boost_state(b);

	if (!cancel_delayed_work_sync(&b->general_unboost)) {
		set_boost_bit(b, GENERAL_BOOST);
		update_online_cpu_policy();
	}

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Set general dynamic stune boost value only
	if no other boosts are active at the moment */
	if (!input_stune_boost_active && !max_stune_boost_active) {
		reset_stune_boost("top-app", boost_slot);
		do_stune_boost("top-app", general_dynamic_stune_boost, &boost_slot);
		general_stune_boost_active = true;
	}	
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */	

	queue_delayed_work(b->wq, &b->general_unboost,
		msecs_to_jiffies(atomic_read(&b->general_boost_dur)));

	set_stune_boost(b, state, GENERAL_STUNE_BOOST, general_stune_boost,
		&b->general_stune_slot);
}

static void general_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b =
		container_of(to_delayed_work(work), typeof(*b), general_unboost);
	u32 state = get_boost_state(b);

	clear_boost_bit(b, GENERAL_BOOST);

#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Reset dynamic stune boost value to the default value 
	only if the boost active is general_dynamic_stune_boost */
	if (general_stune_boost_active) {
		reset_stune_boost("top-app", boost_slot);
		general_stune_boost_active = false;
	}
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

	update_online_cpu_policy();

	clear_stune_boost(b, state, GENERAL_STUNE_BOOST, b->general_stune_slot);
}

static int cpu_notifier_cb(struct notifier_block *nb,
			   unsigned long action, void *data)
{
	struct boost_drv *b = container_of(nb, typeof(*b), cpu_notif);
	struct cpufreq_policy *policy = data;
	u32 boost_freq, min_freq, state;

	if (action != CPUFREQ_ADJUST)
		return NOTIFY_OK;

	state = get_boost_state(b);

	/* Boost CPU to max frequency for max boost */
	if (state & MAX_BOOST) {
		policy->min = policy->max;
		return NOTIFY_OK;
	}

	/*
	 * Boost to policy->max if the boost frequency is higher. When
	 * unboosting, set policy->min to the absolute min freq for the CPU.
	 */
	if (state & INPUT_BOOST || state & GENERAL_BOOST) {
		boost_freq = get_boost_freq(b, policy->cpu, state);
		policy->min = min(policy->max, boost_freq);
	} else {
		min_freq = get_min_freq(b, policy->cpu, state);
		policy->min = max(policy->cpuinfo.min_freq, min_freq);
	}

	return NOTIFY_OK;
}

static int fb_notifier_cb(struct notifier_block *nb,
			  unsigned long action, void *data)
{
	struct boost_drv *b = container_of(nb, typeof(*b), fb_notif);
	struct fb_event *evdata = data;
	int *blank = evdata->data;
	u32 state = get_boost_state(b);

	/* Parse framebuffer blank events as soon as they occur */
	if (action != FB_EARLY_EVENT_BLANK)
		return NOTIFY_OK;

	/* Boost when the screen turns on and unboost when it turns off */
	if (*blank == FB_BLANK_UNBLANK) {
		set_boost_bit(b, SCREEN_AWAKE);
		set_stune_boost(b, state, DISPLAY_STUNE_BOOST, display_stune_boost,
			        &b->display_stune_slot);
	} else {
		clear_boost_bit(b, SCREEN_AWAKE);
		clear_stune_boost(b, state, DISPLAY_STUNE_BOOST,
				  b->display_stune_slot);
		unboost_all_cpus(b);
	}

	return NOTIFY_OK;
}

static void cpu_input_boost_input_event(struct input_handle *handle,
					unsigned int type, unsigned int code,
					int value)
{
	struct boost_drv *b = handle->handler->private;
	last_input_time = jiffies;
	__cpu_input_boost_kick(b);
}

static int cpu_input_boost_input_connect(struct input_handler *handler,
					 struct input_dev *dev,
					 const struct input_device_id *id)
{
	struct input_handle *handle;
	int ret;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpu_input_boost_handle";

	ret = input_register_handle(handle);
	if (ret)
		goto free_handle;

	ret = input_open_device(handle);
	if (ret)
		goto unregister_handle;

	return 0;

unregister_handle:
	input_unregister_handle(handle);
free_handle:
	kfree(handle);
	return ret;
}

static void cpu_input_boost_input_disconnect(struct input_handle *handle)
{
#ifdef CONFIG_DYNAMIC_STUNE_BOOST
	/* Reset dynamic stune boost value to the default value */
	reset_stune_boost("top-app", boost_slot);
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id cpu_input_boost_ids[] = {
	/* Multi-touch touchscreen */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = { [BIT_WORD(ABS_MT_POSITION_X)] =
			BIT_MASK(ABS_MT_POSITION_X) |
			BIT_MASK(ABS_MT_POSITION_Y) }
	},
	/* Touchpad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_KEYBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.keybit = { [BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) },
		.absbit = { [BIT_WORD(ABS_X)] =
			BIT_MASK(ABS_X) | BIT_MASK(ABS_Y) }
	},
	/* Keypad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) }
	},
	{ }
};

static struct input_handler cpu_input_boost_input_handler = {
	.event		= cpu_input_boost_input_event,
	.connect	= cpu_input_boost_input_connect,
	.disconnect	= cpu_input_boost_input_disconnect,
	.name		= "cpu_input_boost_handler",
	.id_table	= cpu_input_boost_ids
};

static int __init cpu_input_boost_init(void)
{
	struct boost_drv *b;
	int ret;

	b = kzalloc(sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;

	b->wq = alloc_workqueue("cpu_input_boost_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (!b->wq) {
		ret = -ENOMEM;
		goto free_b;
	}

	atomic64_set(&b->max_boost_expires, 0);
	INIT_WORK(&b->input_boost, input_boost_worker);
	INIT_DELAYED_WORK(&b->input_unboost, input_unboost_worker);
	INIT_WORK(&b->max_boost, max_boost_worker);
	INIT_DELAYED_WORK(&b->max_unboost, max_unboost_worker);
	INIT_WORK(&b->general_boost, general_boost_worker);
	INIT_DELAYED_WORK(&b->general_unboost, general_unboost_worker);
	atomic_set(&b->state, 0);

	b->cpu_notif.notifier_call = cpu_notifier_cb;
	ret = cpufreq_register_notifier(&b->cpu_notif, CPUFREQ_POLICY_NOTIFIER);
	if (ret) {
		pr_err("Failed to register cpufreq notifier, err: %d\n", ret);
		goto destroy_wq;
	}

	cpu_input_boost_input_handler.private = b;
	ret = input_register_handler(&cpu_input_boost_input_handler);
	if (ret) {
		pr_err("Failed to register input handler, err: %d\n", ret);
		goto unregister_cpu_notif;
	}

	b->fb_notif.notifier_call = fb_notifier_cb;
	b->fb_notif.priority = INT_MAX;
	ret = fb_register_client(&b->fb_notif);
	if (ret) {
		pr_err("Failed to register fb notifier, err: %d\n", ret);
		goto unregister_handler;
	}

	boost_drv_g = b;

	return 0;

unregister_handler:
	input_unregister_handler(&cpu_input_boost_input_handler);
unregister_cpu_notif:
	cpufreq_unregister_notifier(&b->cpu_notif, CPUFREQ_POLICY_NOTIFIER);
destroy_wq:
	destroy_workqueue(b->wq);
free_b:
	kfree(b);
	return ret;
}
late_initcall(cpu_input_boost_init);
