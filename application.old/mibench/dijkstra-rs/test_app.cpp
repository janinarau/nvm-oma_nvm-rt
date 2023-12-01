void run_disjkstra();

void app_init() {
    run_disjkstra();
    asm volatile("svc #0");
}