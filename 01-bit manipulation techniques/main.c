#include <stdio.h>
#include <stdint.h>

// ─── helpers ───────────────────────────────────────────────

void print_bits(const char *label, uint8_t x) {
    printf("%-20s = ", label);
    for (int i = 7; i >= 0; i--)
        printf("%d", (x >> i) & 1);
    printf("  (decimal %d)\n", x);
}

void section(const char *title) {
    printf("\n══ %s ══\n", title);
}

// ─── pattern 1: test, set, clear, toggle ───────────────────

void pattern_bit_ops() {
    section("Pattern 1: Test / Set / Clear / Toggle");

    uint8_t x = 0b10100101;
    int n = 3;
    print_bits("x", x);

    // Test
    int is_set = (x >> n) & 1;   // simpler than comparing to non-zero
    printf("bit %d is: %d\n", n, is_set);

    // Set
    uint8_t after_set = x | (1 << n);
    print_bits("after set   bit 3", after_set);

    // Clear
    uint8_t after_clear = x & ~(1 << n);
    print_bits("after clear bit 3", after_clear);

    // Toggle
    uint8_t after_toggle = x ^ (1 << n);
    print_bits("after toggle bit 3", after_toggle);
}

// ─── pattern 2: clear lowest set bit ──────────────────────

void pattern_clear_lowest() {
    section("Pattern 2: Clear Lowest Set Bit — x & (x-1)");

    uint8_t x = 0b10101100;
    print_bits("x", x);

    uint8_t result = x & (x - 1);
    print_bits("x & (x-1)", result);
}

// ─── pattern 3: isolate lowest set bit ────────────────────

void pattern_isolate_lowest() {
    section("Pattern 3: Isolate Lowest Set Bit — x & (-x)");

    uint8_t x = 0b10101100;
    print_bits("x", x);

    uint8_t result = x & (-x);
    print_bits("x & (-x)", result);
}

// ─── pattern 4: count set bits (popcount) ─────────────────

void pattern_popcount() {
    section("Pattern 4: Count Set Bits (popcount)");

    uint8_t x = 0b10110101;
    print_bits("x", x);

    int count = 0;
    uint8_t tmp = x;
    while (tmp) {
        tmp = tmp & (tmp - 1);  // clear lowest set bit each iteration
        count++;
    }
    printf("set bits: %d\n", count);
}

// ─── pattern 5: power of two check ────────────────────────

void pattern_power_of_two() {
    section("Pattern 5: Power of Two — x & (x-1) == 0");

    uint8_t vals[] = {1, 2, 3, 4, 5, 16, 18};
    for (int i = 0; i < 7; i++) {
        uint8_t v = vals[i];
        int is_pow2 = v && !(v & (v - 1));
        printf("%3d is power of two: %s\n", v, is_pow2 ? "YES" : "NO");
    }
}

// ─── pattern 6: xor tricks ────────────────────────────────

void pattern_xor() {
    section("Pattern 6: XOR Tricks");

    // Swap without temp
    int a = 42, b = 99;
    printf("before swap: a=%d b=%d\n", a, b);
    a = a ^ b;
    b = a ^ b;
    a = a ^ b;
    printf("after  swap: a=%d b=%d\n", a, b);

    // Find unique element
    int arr[] = {4, 1, 2, 1, 2};
    int unique = 0;
    for (int i = 0; i < 5; i++) unique ^= arr[i];
    printf("unique element: %d\n", unique);
}

// ─── pattern 7: bit masks ──────────────────────────────────

void pattern_masks() {
    section("Pattern 7: Masks — pack flags into one int");

    // Permissions packed into one byte
    uint8_t READ  = 1 << 0;   // 0000 0001
    uint8_t WRITE = 1 << 1;   // 0000 0010
    uint8_t EXEC  = 1 << 2;   // 0000 0100

    uint8_t perms = READ | WRITE;
    print_bits("perms (R+W)", perms);

    printf("can read:  %s\n", (perms & READ)  ? "YES" : "NO");
    printf("can write: %s\n", (perms & WRITE) ? "YES" : "NO");
    printf("can exec:  %s\n", (perms & EXEC)  ? "YES" : "NO");

    // Add exec
    perms |= EXEC;
    print_bits("after +EXEC", perms);

    // Remove write
    perms &= ~WRITE;
    print_bits("after -WRITE", perms);
}

// ─── pattern 8: shift arithmetic ──────────────────────────

void pattern_shifts() {
    section("Pattern 8: Shift Arithmetic");

    int x = 1;
    printf("1 << 0  = %-8d  (1 byte)\n",  x << 0);
    printf("1 << 10 = %-8d  (1 KB)\n",    x << 10);
    printf("1 << 20 = %-8d  (1 MB)\n",    x << 20);

    int y = 64;
    printf("\n%d >> 1 = %d  (divide by 2)\n", y, y >> 1);
    printf("%d >> 2 = %d  (divide by 4)\n", y, y >> 2);

    // Alignment check — is x aligned to 8 bytes?
    int addr = 24;
    printf("\naddr %d aligned to 8: %s\n",
           addr, (addr & 7) == 0 ? "YES" : "NO");
}

// ─── main ──────────────────────────────────────────────────

int main() {
    pattern_bit_ops();
    pattern_clear_lowest();
    pattern_isolate_lowest();
    pattern_popcount();
    pattern_power_of_two();
    pattern_xor();
    pattern_masks();
    pattern_shifts();
    return 0;
}