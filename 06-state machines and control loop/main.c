#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_VMS      3
#define LOOP_INTERVAL 2 


//Possible VM states
typedef enum {
    VM_IDLE,
    VM_BOOTING,
    VM_RUNNING,
    VM_STOPPING,
    VM_STOPPED,
    VM_ERROR
} vm_state_t;

// Events that trigger transitions
typedef enum {
    EV_BOOT,
    EV_READY,
    EV_STOP,
    EV_DONE,
    EV_FAIL
} vm_event_t;

typedef struct {
    int          id;
    vm_state_t   actual;    // what is really happening
    vm_state_t   desired;   // what the user wants
    pthread_mutex_t lock;
} vm_t;

vm_t vms[NUM_VMS];

const char *state_name(vm_state_t s) {
    switch (s) {
        case VM_IDLE:     return "IDLE";
        case VM_BOOTING:  return "BOOTING";
        case VM_RUNNING:  return "RUNNING";
        case VM_STOPPING: return "STOPPING";
        case VM_STOPPED:  return "STOPPED";
        case VM_ERROR:    return "ERROR";
        default:          return "UNKNOWN";
    }
}

// Returns the next state given current state + event
// Returns current state if the transition is invalid
vm_state_t transition(vm_state_t current, vm_event_t event) {
    switch (current) {
        case VM_IDLE:
            if (event == EV_BOOT) return VM_BOOTING;
            break;
        case VM_BOOTING:
            if (event == EV_READY) return VM_RUNNING;
            if (event == EV_FAIL)  return VM_ERROR;
            break;
        case VM_RUNNING:
            if (event == EV_STOP) return VM_STOPPING;
            if (event == EV_FAIL) return VM_ERROR;
            break;
        case VM_STOPPING:
            if (event == EV_DONE) return VM_STOPPED;
            if (event == EV_FAIL) return VM_ERROR;
            break;
        case VM_STOPPED:
            if (event == EV_BOOT) return VM_BOOTING;
            break;
        default:
            break;
    }
    return current;
}

//Call this function when we need to apply any specific event by the user or internal functions
void apply_event(vm_t *vm, vm_event_t event) {
    pthread_mutex_lock(&vm->lock);
    vm_state_t next = transition(vm->actual, event);
    if (next != vm->actual) {
        printf("  vm-%d: %s → %s\n",vm->id, state_name(vm->actual), state_name(next));
        vm->actual = next;
    }
    pthread_mutex_unlock(&vm->lock);
}

// Simulating booting of VM
void do_boot(vm_t *vm) {
    printf("  vm-%d: booting hardware...\n", vm->id);
    sleep(1);                    
    apply_event(vm, EV_READY);
}

//Simulating stopping of VM
void do_stop(vm_t *vm) {
    printf("  vm-%d: stopping...\n", vm->id);
    sleep(1);
    apply_event(vm, EV_DONE);
}

// Compare actual to desired to take action ( the main function which the control loop calls)
void reconcile(vm_t *vm) {
    pthread_mutex_lock(&vm->lock);
    vm_state_t actual  = vm->actual;
    vm_state_t desired = vm->desired;
    pthread_mutex_unlock(&vm->lock);

    // Already where we want to be
    if (actual == desired) return;

    printf("vm-%d: actual=%s desired=%s — reconciling\n",vm->id, state_name(actual), state_name(desired));

    // Desired is RUNNING but we haven't started the VM
    if (desired == VM_RUNNING && actual == VM_IDLE) {
        apply_event(vm, EV_BOOT);
        do_boot(vm);
        return;
    }

    // Desired is RUNNING but VM is crashed
    if (desired == VM_RUNNING && actual == VM_ERROR) {
        vm->actual = VM_IDLE;
        apply_event(vm, EV_BOOT);
        do_boot(vm);
        return;
    }

    // Desired is STOPPED but we're running
    if (desired == VM_STOPPED && actual == VM_RUNNING) {
        apply_event(vm, EV_STOP);
        do_stop(vm);
        return;
    }
}

// The control loop runs in its own thread
void *control_loop(void *arg) {
    printf("control loop started\n\n");
    while (1) {
        printf("--- reconcile pass ---\n");
        for (int i = 0; i < NUM_VMS; i++)
            reconcile(&vms[i]);
        printf("--- pass done ---\n\n");
        sleep(LOOP_INTERVAL);
    }
    return NULL;
}

void vm_init(vm_t *vm, int id, vm_state_t desired) {
    vm->id      = id;
    vm->actual  = VM_IDLE;
    vm->desired = desired;
    pthread_mutex_init(&vm->lock, NULL);
}

int main() {

    // Three VMs
    vm_init(&vms[0], 1, VM_RUNNING);
    vm_init(&vms[1], 2, VM_RUNNING);
    vm_init(&vms[2], 3, VM_RUNNING);

    // Start the control loop in background
    pthread_t loop_thread;
    pthread_create(&loop_thread, NULL, control_loop, NULL);

    // Simulate vm-2 crashing after 4 seconds
    sleep(4);
    printf("\n>>> vm-2 crashed!\n\n");
    apply_event(&vms[1], EV_FAIL);

    // Simulate user stopping vm-3 after 6 seconds
    sleep(2);
    printf("\n>>> user requests vm-3 stop\n\n");
    vms[2].desired = VM_STOPPED;

    // Let the control loop react
    sleep(6);
    pthread_cancel(loop_thread);
    return 0;
}