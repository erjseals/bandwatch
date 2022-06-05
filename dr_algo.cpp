void dynamic_regulation()
{
  if (cpu_utilization > cpu_threshold) {
    // Throttle GPU relative to CPU MC Utilization
    throttle_amount = cpu_utilization * THROTTLE_MAX
                      / MAX_CPU_UTILIZATION;  
    throttle_cpus();
  }
  else {
    throttle_amount = 0;
    relax_cpus();
  }
  set_throttle(throttle_amount);
}
