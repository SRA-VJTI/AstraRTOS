#include "system_init.h"
#include "task.h"
#include "heap.h"
#include "mutex.h"
#include "uart.h"
#include "stats.h"

#define NUM_PHILOSOPHER 5
#define NUM_FORKS NUM_PHILOSOPHER
#define STAT_PRINT_DELAY 500

typedef enum {
    THINKING,
    HUNGRY,
    HOLDING_ONE,
    EATING
} philosohper_state;

static os_mutex_t forks[NUM_FORKS];
static os_tcb_t *philosopher_tasks[NUM_PHILOSOPHER];
volatile philosohper_state states[NUM_PHILOSOPHER];
volatile uint32_t state_start_time[NUM_PHILOSOPHER];
volatile uint32_t state_duration[NUM_PHILOSOPHER];
volatile uint32_t meals[NUM_PHILOSOPHER];

static int get_philosopher_id(void) {
    for(int i = 0; i < NUM_PHILOSOPHER; i++) {
        if(philosopher_tasks[i] == os_current_task_ptr) {
            return i;
        }
    }
    return -1;
}

static inline void get_forks(int philosopher, int *low, int *high) {
    int left = philosopher;
    int right = (philosopher + 1) % NUM_PHILOSOPHER;
    if(left < right) {
        *low = left;
        *high = right;
    }
    else {
        *low = right;
        *high = left;
    }
}

static uint32_t get_random_delay(int id, uint32_t period) {
    uint32_t delay = (((system_ticks) / 100) * (id + 1)) & 0x1F;
    delay = (delay + 10) * period;
    return delay;
}

static const char *state_name(philosohper_state s) {
    switch(s) {
    case THINKING:
        return "THINKING";
    case HUNGRY:
        return "HUNGRY";
    case HOLDING_ONE:
        return "HOLDING ONE FORK";
    case EATING:
        return "EATING";
    }
    return "??";
}

static void uint_to_str(uint32_t value, char *buf) {
    char tmp[12];
    int i = 0;

    if(value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while(value > 0) {
        tmp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    int j = 0;
    while(i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

static void set_philosopher_state(int id, philosohper_state new) {
    uint32_t current_ticks = system_ticks;
    state_duration[id] = current_ticks - state_start_time[id];
    state_start_time[id] = current_ticks;
    states[id] = new;
}

void philosopher_task(void) {
    int id = get_philosopher_id();
    if(id < 0)
        return;
    int low, high;
    get_forks(id, &low, &high);
    state_start_time[id] = system_ticks;
    set_philosopher_state(id, THINKING);
    while(1) {
        set_philosopher_state(id, THINKING);
        os_delay(get_random_delay(id, 25));
        set_philosopher_state(id, HUNGRY);
        os_mutex_take(&forks[low]);
        set_philosopher_state(id, HOLDING_ONE);
        os_mutex_take(&forks[high]);
        set_philosopher_state(id, EATING);
        meals[id]++;
        os_delay(get_random_delay(id, 25));
        os_mutex_give(&forks[high]);
        os_mutex_give(&forks[low]);
    }
}

void print_philosopher_stats(void) {
    while(1) {
        char num_str[12];
        uart_send_string(USART2_BASE, "Printing All Philosopher's Statistics \r\n");
        for(int i = 0; i < NUM_PHILOSOPHER; i++) {
            uart_send_string(USART2_BASE, "Philosopher ");
            uint_to_str(i, num_str);
            uart_send_string(USART2_BASE, num_str);
            uart_send_string(USART2_BASE, " : ");
            uart_send_string(USART2_BASE, (char *)state_name(states[i]));
            uart_send_string(USART2_BASE, " , Meals : ");
            uint_to_str(meals[i], num_str);
            uart_send_string(USART2_BASE, num_str);
            uart_send_string(USART2_BASE, " , State Duration : ");
            uint_to_str(state_duration[i], num_str);
            uart_send_string(USART2_BASE, num_str);
            uart_send_string(USART2_BASE, "  \r\n");
        }
        uart_send_string(USART2_BASE, "\r\n");
        os_delay(STAT_PRINT_DELAY);
    }
}

int main() {
    uart_config_t config = {.baud_rate = 9600,
                            .data_bits = 8,
                            .parity = UART_PARITY_DISABLE,
                            .stop_bits = UART_STOPBITS_1,
                            .mode = UART_MODE_TX_RX,
                            .irq_flags = UART_IRQ_RXNE};
    system_init();
    uart_init(USART2_BASE, &config);
    os_heap_init();
    for(int i = 0; i < NUM_FORKS; i++) {
        os_mutex_init(&forks[i]);
    }
    for(int i = 0; i < NUM_PHILOSOPHER; i++) {
        meals[i] = 0;
        state_duration[i] = 0;
        philosopher_tasks[i] = os_task_create(philosopher_task, i + 2, 512);
    }
    os_task_create(print_philosopher_stats, 7, 512);
    os_start();
    while(1) {
    }
}
