void dynamic_regulation(int rt_core_bandwidth)
{
  int mc_cpu_utilization = mc_cpu_activity_monitor();

  if (rt_core_bandwidth > active_threshold) {
    throttle_nrt_cpu_cores();
    throttle_gpu(mc_cpu_utilization);
  }
  else {
    unthrottle_nrt_cpu_cores();
    throttle_gpu(0);
  }
}

void throttle_gpu(int mc_cpu_utilization) 
{
  throttle_level = mc_cpu_utilization * MAX_THROTTLE
                   / MAX_CPU_UTILIZATION;
  set_gpu_throttle(throttle_level);
}
