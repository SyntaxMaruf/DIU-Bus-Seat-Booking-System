#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_BUSES 12
#define NUM_SEATS 40
#define NUM_STATIONS 11
#define NUM_TRIPS 30
#define MAX_BOOKINGS 20000
#define ADVANCE_FEE 10
#define FARE_PER_KM 2.60
#define BOOKING_CUTOFF 5 // minutes before bus arrival time
#define BOOKING_FILE "orchid_bookings.txt" // File to store booking data

typedef struct {
    int ticketNo, date, dir, trip, seat, lo, hi, ptype, active;
    char name[60];
} Booking;

const char *stations[NUM_STATIONS] = {
    "DIU", "Birulia", "Ponchobuti", "Metro Uttara Center (MUC)",
    "Metro Uttara South (MUS)", "Mirpur 12", "Mirpur 11",
    "Mirpur 10", "Agargaon", "Bijoy Sarani", "Farmgate"
};

const int segmentMinutes[NUM_STATIONS - 1] = {
    20, 10, 5, 5, 5, 5, 5, 10, 5, 10
};

/* Approximate project distances, not verified road distances. */
const double segmentDistanceKm[NUM_STATIONS - 1] = {
    4.0, 3.0, 4.5, 1.6, 4.0, 1.1, 1.2, 3.6, 1.9, 0.7
};

Booking bookings[MAX_BOOKINGS];
int bookingCount = 0;
int nextTicketNo = 1001;

int askInt(const char *prompt, int min, int max) {
    char line[100], extra;
    int value;
    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) exit(0);
        if (sscanf(line, " %d %c", &value, &extra) == 1 &&
            value >= min && value <= max) return value;
        printf("Invalid input. Enter a number from %d to %d.\n", min, max);
    }
}

void askText(const char *prompt, char *out, int size) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(out, size, stdin)) exit(0);
        out[strcspn(out, "\n")] = '\0';
        if (strlen(out) > 0) return;
        printf("Name cannot be empty.\n");
    }
}

int dateKey(int d, int m, int y) {
    return y * 10000 + m * 100 + d;
}

int daysInMonth(int m, int y) {
    int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
        return 29;
    return days[m - 1];
}

int todayKey(int *nowMinutes) {
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    *nowMinutes = lt->tm_hour * 60 + lt->tm_min;
    return dateKey(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
}

int askDate(int today) {
    char line[100], extra;
    int d, m, y, key;
    while (1) {
        printf("Travel date (DD MM YYYY): ");
        if (!fgets(line, sizeof(line), stdin)) exit(0);
        if (sscanf(line, "%d %d %d %c", &d, &m, &y, &extra) != 3 ||
            m < 1 || m > 12 || y < 2000 ||
            d < 1 || d > daysInMonth(m, y)) {
            printf("Invalid date.\n");
            continue;
        }
        key = dateKey(d, m, y);
        if (key < today) {
            printf("Past dates are not allowed.\n");
            continue;
        }
        return key;
    }
}

void printDate(int key) {
    printf("%02d-%02d-%04d", key % 100, (key / 100) % 100, key / 10000);
}

void formatTime(int minutes, char *out) {
    int h = (minutes / 60) % 24, m = minutes % 60;
    int h12 = h % 12;
    if (h12 == 0) h12 = 12;
    sprintf(out, "%02d:%02d %s", h12, m, h >= 12 ? "PM" : "AM");
}

int travelMinutes(int from, int to) {
    int i, total = 0;
    if (from > to) {
        int temp = from; from = to; to = temp;
    }
    for (i = from; i < to; i++) total += segmentMinutes[i];
    return total;
}

double travelDistance(int from, int to) {
    int i, start, end;
    double total = 0.0;

    if (from > to) {
        start = to;
        end = from;
    } else {
        start = from;
        end = to;
    }

    for (i = start; i < end; i++)
        total += segmentDistanceKm[i];

    return total;
}

int calculateFare(int from, int to) {
    int fare;
    fare = (int)(travelDistance(from, to) * FARE_PER_KM);
    fare = (fare / 5) * 5;
    if (fare < 10) fare = 10;
    return fare;
}

int stationOffset(int dir, int station) {
    if (dir == 0) return travelMinutes(0, station);
    return travelMinutes(station, NUM_STATIONS - 1);
}

int busArrival(int dir, int trip, int station) {
    return 420 + trip * 30 + stationOffset(dir, station);
}

const char *busName(int dir, int trip) {
    static char name[20];
    int number = ((trip + (dir ? 6 : 0)) % NUM_BUSES) + 1;
    sprintf(name, "ORCHID-%02d", number);
    return name;
}

int isReservedSeat(int seat) {
    /* 8 reserved seats: first two seats of the first four rows */
    return seat == 1 || seat == 2 || seat == 5 || seat == 6 ||
           seat == 9 || seat == 10 || seat == 13 || seat == 14;
}

int canUseSeat(int seat, int ptype) {
    return !(ptype == 1 && isReservedSeat(seat));
}

void loadBookings(void) {
    FILE *fp = fopen(BOOKING_FILE, "r");
    if (!fp) return;
    while (bookingCount < MAX_BOOKINGS) {
        Booking b;
        int got = fscanf(fp, "%d %d %d %d %d %d %d %d %d %59[^\n]",
                         &b.ticketNo, &b.date, &b.dir, &b.trip, &b.seat,
                         &b.lo, &b.hi, &b.ptype, &b.active, b.name);
        if (got != 10) break;
        bookings[bookingCount++] = b;
        if (b.ticketNo >= nextTicketNo) nextTicketNo = b.ticketNo + 1;
    }
    fclose(fp);
}

void saveAllBookings(void) {
    int i;
    FILE *fp = fopen(BOOKING_FILE, "w");
    if (!fp) {
        printf("Error: Could not update booking file.\n");
        return;
    }
    for (i = 0; i < bookingCount; i++) {
        Booking *b = &bookings[i];
        fprintf(fp, "%d %d %d %d %d %d %d %d %d %s\n",
                b->ticketNo, b->date, b->dir, b->trip, b->seat,
                b->lo, b->hi, b->ptype, b->active, b->name);
    }
    fclose(fp);
}

int seatIsFree(int date, int dir, int trip, int seat, int lo, int hi) {
    int i;
    for (i = 0; i < bookingCount; i++) {
        Booking *b = &bookings[i];
        if (b->active && b->date == date && b->dir == dir &&
            b->trip == trip && b->seat == seat &&
            b->lo < hi && lo < b->hi) return 0;
    }
    return 1;
}

void showStations(void) {
    int i;
    printf("\n----------- ROUTE STATIONS -----------\n");
    for (i = 0; i < NUM_STATIONS; i++)
        printf("%2d. %s\n", i + 1, stations[i]);
}

void showSeatMap(int date, int dir, int trip, int lo, int hi,
                 int *selected, int selectedCount) {
    int row, col, seat;
    printf("\n          FRONT / DRIVER\n");
    printf("         LEFT       RIGHT\n");
    for (row = 0; row < 10; row++) {
        printf("       ");
        for (col = 0; col < 4; col++) {
            seat = row * 4 + col + 1;
            if (col == 2) printf("   ");
            if (selectedCount > 0) {
                int j, found = 0;
                for (j = 0; j < selectedCount; j++)
                    if (selected[j] == seat) found = 1;
                if (found) { printf("[**]"); continue; }
            }
            if (!seatIsFree(date, dir, trip, seat, lo, hi))
                printf("[XX]");
            else if (isReservedSeat(seat))
                printf("(%2d)", seat);
            else
                printf("[%2d]", seat);
        }
        printf("\n");
    }
    printf("\n[XX] Booked   [**] Selected  \n(number)->Female/Faculty reserved.\n");
}

void askPassenger(char *name, int *ptype) {
    int category;
    category = askInt("1. Student  2. Faculty: ", 1, 2);
    if (category == 2) *ptype = 3;
    else *ptype = askInt("1. Male student  2. Female student: ", 1, 2);
    askText("Traveler name: ", name, 60);
}

int askRoute(int dir, int *from, int *to) {
    while (1) {
        *from = askInt("Boarding station number: ", 1, NUM_STATIONS) - 1;
        *to = askInt("Destination station number: ", 1, NUM_STATIONS) - 1;
        if (dir == 0 && *to <= *from) {
            printf("Destination must be after boarding station toward Farmgate.\n");
            continue;
        }
        if (dir == 1 && *to >= *from) {
            printf("Destination must be before boarding station toward DIU.\n");
            continue;
        }
        return 1;
    }
}

int chooseTrip(int dir, int board, int date, int now, int mode, int *tripOut) {
    int trips[NUM_TRIPS], count = 0, k, arrival;
    char timeText[20];
    printf("\nAvailable buses at %s:\n", stations[board]);
    for (k = 0; k < NUM_TRIPS; k++) {
        arrival = busArrival(dir, k, board);
        if (mode == 0 && date == todayKey(&now) &&
            now > arrival - BOOKING_CUTOFF) continue;
        if (mode == 1 && (arrival < now - 10 || arrival > now + 30))
            continue;
        trips[count++] = k;
        formatTime(arrival, timeText);
        printf("%2d. %s | %s\n", count, timeText, busName(dir, k));
    }
    if (count == 0) {
        printf("No bus available for the selected time.\n");
        return 0;
    }
    *tripOut = trips[askInt("Select bus: ", 1, count) - 1];
    return 1;
}

void printReceipt(int ticketNo, const char *name, int ptype, int date,
                  int dir, int trip, int from, int to,
                  int *seats, int count, int advanceFee, int fare) {
    int i, arrival = busArrival(dir, trip, from);
    char timeText[20];
    formatTime(arrival, timeText);
    printf("\n================================================\n");
    printf("             DIU BUS - ORCHID TICKET\n");
    printf("================================================\n");
    printf("Ticket No : %d\nTraveler : %s\n", ticketNo, name);
    printf("Traveler type: %s\n",
           ptype == 1 ? "Male Student" :
           ptype == 2 ? "Female Student" : "Faculty");
    printf("Travel date: ");
    printDate(date);
    printf("\nBus       : %s\n", busName(dir, trip));
    printf("Direction : %s\n", dir == 0 ? "DIU -> Farmgate" : "Farmgate -> DIU");
    printf("Bus time  : %s at %s\n", timeText, stations[from]);
    printf("From      : %s\nTo        : %s\n", stations[from], stations[to]);
    printf("Seat(s)   : ");
    for (i = 0; i < count; i++)
        printf("%d%s", seats[i], i == count - 1 ? "\n" : ", ");
    printf("------------------------------------------------\n");
    printf("Advance booking fee: %d BDT\n", advanceFee);
    printf("Fare payable on bus: %d BDT\n", fare * count);
    printf("------------------------------------------------\n");
    printf("Total amount       : %d BDT\n", advanceFee + fare * count);
    printf("================================================\n");
    printf("      Thank you for choosing our service!\n");
    printf("================================================\n");
}

void bookTicket(void) {
    char name[60];
    int ptype, now, today, date, dir, from, to, trip, lo, hi;
    int count, seats[NUM_SEATS], i, seat, fare, ticketNo;
    int fee, confirm, stationLo, stationHi;
    today = todayKey(&now);
    printf("\n========== BOOK SEAT & BUY TICKET ==========\n");
    askPassenger(name, &ptype);
    date = askDate(today);
    dir = askInt("Going towards: 1. Farmgate  2. DIU: ", 1, 2) - 1;
    showStations();
    askRoute(dir, &from, &to);

    if (!chooseTrip(dir, from, date, now, 0, &trip)) return;

    stationLo = dir == 0 ? from : NUM_STATIONS - 1 - from;
    stationHi = dir == 0 ? to : NUM_STATIONS - 1 - to;
    lo = stationLo < stationHi ? stationLo : stationHi;
    hi = stationLo < stationHi ? stationHi : stationLo;

    count = askInt("How many seats : ", 1, NUM_SEATS);
    for (i = 0; i < count; i++) {
        showSeatMap(date, dir, trip, lo, hi, seats, i);
        while (1) {
            char prompt[80];
            sprintf(prompt, "Enter seat number for passenger %d: ", i + 1);
            seat = askInt(prompt, 1, NUM_SEATS);
            if (canUseSeat(seat, ptype) == 0) {
                printf("This seat is reserved for Female Students and Faculty only.\n");
                continue;
            }
            if (!seatIsFree(date, dir, trip, seat, lo, hi)) {
                printf("This seat is already booked. Choose another seat.\n");
                continue;
            }
            {
                int j, duplicate = 0;
                for (j = 0; j < i; j++)
                    if (seats[j] == seat) duplicate = 1;
                if (duplicate) {
                    printf("You already selected this seat.\n");
                    continue;
                }
            }
            break;
        }
        seats[i] = seat;
    }

    fee = date == today ? 0 : ADVANCE_FEE * count;
    fare = calculateFare(from, to);
    printf("\nRoute distance: %.1f KM\n", travelDistance(from, to));
    printf("Fare per seat: %d BDT\n", fare);
    printf("Advance booking fee: %d BDT per seat x %d = %d BDT\n",
           date == today ? 0 : ADVANCE_FEE, count, fee);
    printf("Main travel fare: %d x %d = %d BDT (pay on the bus).\n",
           fare, count, fare * count);
    confirm = askInt("Press 1 to confirm booking, 0 to cancel: ", 0, 1);
    if (!confirm) {
        printf("Booking cancelled.\n");
        return;
    }
    if (bookingCount + count > MAX_BOOKINGS) {
        printf("Booking storage is full.\n");
        return;
    }

    ticketNo = nextTicketNo++;
    for (i = 0; i < count; i++) {
        Booking *b = &bookings[bookingCount++];
        b->ticketNo = ticketNo;
        b->date = date; b->dir = dir; b->trip = trip; b->seat = seats[i];
        b->lo = lo; b->hi = hi; b->ptype = ptype; b->active = 1;
        strcpy(b->name, name);
    }
    saveAllBookings();
    printReceipt(ticketNo, name, ptype, date, dir, trip, from, to,
                 seats, count, fee, fare);
}

/* Check whether at least one bus is available anywhere right now.
   The same time window is used by chooseTrip(): 10 minutes late to
   30 minutes ahead of the current time. */
int hasAnyBusAvailableNow(int now) {
    int dir, station, trip, arrival;

    for (dir = 0; dir <= 1; dir++) {
        for (station = 0; station < NUM_STATIONS; station++) {
            for (trip = 0; trip < NUM_TRIPS; trip++) {
                arrival = busArrival(dir, trip, station);
                if (arrival >= now - 10 && arrival <= now + 30) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void payOnBus(void) {
    char name[60], prompt[80];
    int ptype, now, date, dir, from, to, trip, lo, hi;
    int stationLo, stationHi, count, seats[NUM_SEATS];
    int i, j, seat, duplicate, fare, confirm, ticketNo;

    date = todayKey(&now);
    printf("\n========== PAY FARE ON BUS (NO BOOKING) ==========\n");

    if (!hasAnyBusAvailableNow(now)) {
        printf("\nNo bus available right now. Please try again later.\n");
        return;
    }

    askPassenger(name, &ptype);
    dir = askInt("Bus going towards: 1. Farmgate  2. DIU: ", 1, 2) - 1;
    showStations();
    askRoute(dir, &from, &to);
    if (!chooseTrip(dir, from, date, now, 1, &trip)) return;

    stationLo = dir == 0 ? from : NUM_STATIONS - 1 - from;
    stationHi = dir == 0 ? to : NUM_STATIONS - 1 - to;
    lo = stationLo < stationHi ? stationLo : stationHi;
    hi = stationLo < stationHi ? stationHi : stationLo;

    count = askInt("How many seats did you take? : ", 1, NUM_SEATS);
    for (i = 0; i < count; i++) {
        while (1) {
            sprintf(prompt, "Enter seat number for passenger %d: ", i + 1);
            seat = askInt(prompt, 1, NUM_SEATS);

            if (!canUseSeat(seat, ptype)) {
                printf("This seat is reserved for Female Students and Faculty only.\n");
                continue;
            }
            if (!seatIsFree(date, dir, trip, seat, lo, hi)) {
                printf("This seat is already booked. Choose another seat.\n");
                continue;
            }

            duplicate = 0;
            for (j = 0; j < i; j++)
                if (seats[j] == seat) duplicate = 1;
            if (duplicate) {
                printf("You already selected this seat. Choose another seat.\n");
                continue;
            }
            break;
        }
        seats[i] = seat;
    }

    fare = calculateFare(from, to);
    printf("\nRoute distance: %.1f KM\n", travelDistance(from, to));
    printf("Fare per seat: %d BDT\n", fare);
    printf("Total fare: %d x %d = %d BDT\n", fare, count, fare * count);
    printf("Advance booking fee: 0 BDT\n");
    confirm = askInt("Press 1 to confirm, 0 to cancel: ", 0, 1);
    if (!confirm) {
        printf("Booking cancelled.\n");
        return;
    }
    if (bookingCount + count > MAX_BOOKINGS) {
        printf("Booking storage is full.\n");
        return;
    }

    ticketNo = nextTicketNo++;
    for (i = 0; i < count; i++) {
        Booking *b = &bookings[bookingCount++];
        b->ticketNo = ticketNo;
        b->date = date;
        b->dir = dir;
        b->trip = trip;
        b->seat = seats[i];
        b->lo = lo;
        b->hi = hi;
        b->ptype = ptype;
        b->active = 1;
        strcpy(b->name, name);
    }

    saveAllBookings();
    printReceipt(ticketNo, name, ptype, date, dir, trip, from, to,
                 seats, count, 0, fare);
}

void cancelTicket(void) {
    int ticketNo, i, found = 0, count = 0;
    printf("\n========== CANCEL TICKET ==========\n");
    ticketNo = askInt("Enter ticket number: ", 1, 999999999);
    for (i = 0; i < bookingCount; i++) {
        if (bookings[i].ticketNo == ticketNo && bookings[i].active) {
            found = 1;
            count++;
        }
    }
    if (!found) {
        printf("Active ticket not found.\n");
        return;
    }
    printf("Ticket found. Seats under this ticket: ");
    for (i = 0; i < bookingCount; i++)
        if (bookings[i].ticketNo == ticketNo && bookings[i].active)
            printf("%d ", bookings[i].seat);
    printf("\n");
    if (!askInt("Confirm cancellation? 1. Yes  0. No: ", 0, 1)) {
        printf("Cancellation stopped.\n");
        return;
    }
    for (i = 0; i < bookingCount; i++)
        if (bookings[i].ticketNo == ticketNo)
            bookings[i].active = 0;
    saveAllBookings();
    printf("\n=============== CANCELED TICKET ================\n");
    printf("\nSeat/Ticket Successfully Cancelled.\n");
    printf("Ticket number: %d\n", ticketNo);
    printf("Cancelled seats: ");
    for (i = 0; i < bookingCount; i++)
        if (bookings[i].ticketNo == ticketNo)
            printf("%d ", bookings[i].seat);
    printf("\n\nThank you for choosing our service.\nWe hope to see you again!\n");
    printf("\n================================================\n");
}

void showMenu(void) {
    printf("\n================================================\n");
    printf("          DIU BUS SEAT BOOKING SYSTEM\n");
    printf("================================================\n");
    printf("1. Book Seat & Buy Ticket\n");
    printf("2. Pay Fare On Bus (no booking)\n");
    printf("3. Cancel Ticket\n");
    printf("4. Exit\n");
    printf("================================================\n");
}

int main(void) {
    int choice;
    loadBookings();
    while (1) {
        showMenu();
        choice = askInt("Enter choice: ", 1, 4);
        if (choice == 1) bookTicket();
        else if (choice == 2) payOnBus();
        else if (choice == 3) cancelTicket();
        else {
            printf("\nThank You For Using Our Service!\n\n");
            break;
        }
    }
    return 0;
}
