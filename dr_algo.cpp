void dynamic_regulation()
{
if (cpu_utilization > cpu_threshold) {
  // Monitor GPU
  if (gpu_utilization > gpu_threshold_l)
    increase_throttle();
  else 
    decrease_throttle();
  
  // Monitor CPUs
  if (RT_core_bandwidth > rt_threshold)
    throttle_cpus();
  else
    relax_cpus();
}
else {
  if (gpu_utilization > gpu_threshold_h)
    increase_throttle();
  else 
    decrease_throttle();

  relax_cpus();
}
}
