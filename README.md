# 🚌 DIU Bus Seat Booking System in C

> 🎫 **Choose your route, pick your seats, and get your ticket from the terminal.**

[![Maruf Billah — Visit My Portfolio](portfolio-banner.svg)](https://syntaxmaruf-portfolio.curlyriver4.chatgpt.site/)

## 👋 About Me
🎓 I am **Maruf Billah**, a CSE student at **Daffodil International University**. I built this project in my **1st year, 3rd semester**, using my own ideas with AI assistance.

## 🤖 My Idea: Learn the Logic, Build with AI
💡 I wanted to turn a daily travel problem into a useful C project. I used AI to help develop the code and explain possible solutions. My main focus is the program logic: what a user needs, which rules should apply, and how each part works.

🧠 AI helps me build and learn. I am still learning C, and I aim to understand the main flow, check the results, and explain my choices clearly.

## 🎯 What Does This Project Do?
🚌 This program models ORCHID bus trips between **DIU and Farmgate**. A passenger can book seats, record a same-day ride through **Pay Fare On Bus**, or cancel a ticket.

📌 This is a student project. The routes, times, fares, and seat rules are demo rules in the code. This is not an official DIU booking or payment service.

## ✨ Main Features
| | Feature | What it does |
| :-: | :--- | :--- |
| 🎫 | Book several seats | Select 1 to 40 seats under one ticket number. |
| 🗺️ | Travel both ways | Choose DIU to Farmgate or Farmgate to DIU. |
| 🚏 | Choose your stops | Pick a boarding station and a destination in the right direction. |
| 💺 | See the seat map | View 40 seats in 10 rows, with 4 seats in each row. |
| 🔄 | Reuse seats along a route | Allow the same seat on parts of a trip that do not overlap. |
| 👩‍🎓 | Reserved seats | Keep 8 seats available to female students and faculty. |
| 🕒 | Time checks | Filter buses using the selected travel mode and the computer clock. |
| 🧮 | Distance-based fare | Calculate fare from the route distances stored in the code. |
| 📅 | Future-date booking | Add BDT 10 per seat for a future-date reservation. |
| 💵 | Pay fare on bus | Record a same-day ride with no advance fee. |
| 🧾 | Ticket receipt | Show passenger details, route, seats, time, and cost. |
| ❌ | Cancel a ticket | Cancel all active seats under one ticket after confirmation. |
| 💾 | Save records | Store bookings in a local text file and load them next time. |

## 🖥️ Main Menu
```text
1. Book Seat & Buy Ticket
2. Pay Fare On Bus (no booking)
3. Cancel Ticket
4. Exit
```

## 🗺️ Route Stations
| No. | Station |
| :---: | :--- |
| 1 | DIU |
| 2 | Birulia |
| 3 | Ponchobuti |
| 4 | Metro Uttara Center (MUC) |
| 5 | Metro Uttara South (MUS) |
| 6 | Mirpur 12 |
| 7 | Mirpur 11 |
| 8 | Mirpur 10 |
| 9 | Agargaon |
| 10 | Bijoy Sarani |
| 11 | Farmgate |

↔️ The program uses the same station list for both directions. It checks that the destination is after the boarding stop in the chosen direction.

## 🔄 How a Booking Works
👤 **1. Enter passenger details.** Choose student or faculty. A student also selects male or female. Enter the traveler name.

📅 **2. Enter the date.** Use `DD MM YYYY`. The program checks month length and leap years. Past dates are rejected.

🚏 **3. Choose the route.** Select a direction, boarding station, and destination.

🚌 **4. Choose a trip.** The program shows matching bus times at the boarding station.

💺 **5. Choose seats.** It checks reserved seats, existing bookings, and repeated seat numbers in your selection.

🧮 **6. Review the cost.** It shows route distance, fare per seat, and any advance fee.

✅ **7. Confirm.** Enter `1` to save the booking or `0` to stop. A confirmed group of seats gets one ticket number.

## 💺 What Makes the Seat Logic Special?
🔍 A seat check uses the **date, direction, trip, seat number, and route section**. The bus name alone is not the booking key.

💡 For example, one passenger can use seat 15 from **DIU to Mirpur 10**. Another can use seat 15 from **Mirpur 10 to Farmgate** on the same trip. They use different parts of the route.

🚫 A request for that seat from **Mirpur 12 to Farmgate** would overlap with the first passenger's trip, so it is blocked.

🧩 The code stores each route section using `lo` and `hi`. Two sections overlap when:
```c
existing.lo < requested.hi && requested.lo < existing.hi
```

📍 Trips that meet at one stop can share a seat. Trips that use the same section cannot.

## 👩‍🎓 Reserved Seats
💺 The reserved seat numbers are **1, 2, 5, 6, 9, 10, 13, and 14**.

👥 Female students and faculty can choose these seats or other free seats. Male students can choose free seats outside this reserved group.

📌 One passenger category and one traveler name apply to every seat under a ticket. Mixed passenger groups do not have separate categories per seat.

## 🕒 Bus Times and Booking Rules
| | Rule | Value in the code |
| :-: | :--- | :--- |
| 🚌 | Bus names | ORCHID-01 to ORCHID-12, assigned in a cycle. |
| 🗓️ | Trip slots | 30 slots per direction. |
| 🌅 | First origin departure | 7:00 AM. |
| ⏱️ | Gap between origin departures | 30 minutes. |
| 🌙 | Last origin departure | 9:30 PM. |
| 🚏 | Arrival at a stop | Origin departure plus the stored travel time to that stop. |
| 🎫 | Same-day reservation cutoff | Trips are hidden when the current time is later than 5 minutes before arrival. |
| 💵 | Pay-on-bus window | Arrival from 10 minutes before now to 30 minutes after now. |

🕰️ The program uses the computer's local date and time. These are fixed demo schedules, not live bus locations. The ORCHID name cycle does not model the full movement of each physical bus.

🔍 Pay Fare On Bus first checks whether any trip at any station is in the time window. The chosen boarding station may still have no matching trip.

## 🧮 How the Fare Is Calculated
📏 Add the stored distances between the boarding and destination stops.

💵 Multiply that distance by **BDT 2.60 per km**. The code then rounds down to a multiple of BDT 5 and applies a minimum fare of **BDT 10 per seat**.

📅 A reservation for a future date adds **BDT 10 per seat**. A same-day reservation has no advance fee.

| | Cost | Calculation |
| :-: | :--- | :--- |
| 💺 | Travel fare | Fare per seat × number of seats |
| 📅 | Future-date advance fee | BDT 10 × number of seats |
| 💰 | Total shown | Travel fare + advance fee |

🧾 **Example: DIU to Farmgate, 2 seats on a future date.** The stored distance is 25.6 km. The raw fare is BDT 66.56 per seat. The program rounds it down to BDT 65. Travel fare is BDT 130, the advance fee is BDT 20, and the total shown is **BDT 150**.

📌 These distances are project estimates. They are not verified road distances or official DIU fares.

## 💵 What Does Pay Fare On Bus Mean?
🚌 This option records a same-day ride. It uses the nearby-time window, checks the selected seats, and creates a ticket with **zero advance fee**.

📝 Although the menu says “no booking,” this option still saves seat records. Later seat checks can see them.

💳 The program only shows amounts and records the ride. It does not collect money, verify a payment, or connect to a payment gateway.

## ❌ How Cancellation Works
🔢 Enter the ticket number. The program finds the active seats under it and lists them.

✅ Confirm with `1` to cancel, or enter `0` to keep the ticket.

🔓 Cancellation sets those records to inactive. Their seats become available for later checks. Old records stay in the file; they are not deleted.

📌 There is no refund calculation or payment refund in this version.

## 🧱 Data and C Concepts
| | Item | Why it is used |
| :-: | :--- | :--- |
| 🧱 | `Booking` structure | Keep ticket, date, trip, seat, route section, passenger type, name, and active state together. |
| 📚 | Arrays | Hold station names, route times, distances, and booking records. |
| 🔁 | Loops | Show menus, inspect records, and collect several seats. |
| 🔀 | Conditions | Apply route, date, seat, time, and fare rules. |
| 🧩 | Functions | Split input, route checks, booking, receipts, and saving into smaller jobs. |
| 📍 | Pointers | Pass results between functions and access booking records. |
| 🔤 | Strings | Store traveler names and show bus names. |
| 🕰️ | `time.h` | Read the computer's current date and time. |
| 💾 | File handling | Save and load the local booking list. |

## 🛠️ Main Functions
| Function | Simple explanation |
| :--- | :--- |
| ⌨️ `askInt()`, `askText()` | Read menu numbers and traveler names. |
| 📅 `dateKey()`, `daysInMonth()`, `todayKey()`, `askDate()` | Build, check, and compare dates. |
| 🕒 `printDate()`, `formatTime()` | Show readable dates and times. |
| 📏 `travelMinutes()`, `travelDistance()` | Add time or distance between stops. |
| 💵 `calculateFare()` | Calculate the fare for one seat. |
| 🚏 `stationOffset()`, `busArrival()`, `busName()` | Work out stop arrival times and bus labels. |
| 👥 `isReservedSeat()`, `canUseSeat()` | Apply the reserved-seat rule. |
| 💺 `seatIsFree()`, `showSeatMap()` | Check route overlap and display seats. |
| 🧭 `showStations()`, `askPassenger()`, `askRoute()` | Collect passenger and route choices. |
| 🚌 `chooseTrip()`, `hasAnyBusAvailableNow()` | Find trips in the allowed time range. |
| 🎫 `bookTicket()`, `payOnBus()` | Create confirmed seat records. |
| 🧾 `printReceipt()` | Print the ticket and cost details. |
| ❌ `cancelTicket()` | Mark a ticket's seats as inactive. |
| 💾 `loadBookings()`, `saveAllBookings()` | Read and write the booking file. |
| 🚀 `showMenu()`, `main()` | Run the menu and call the chosen action. |

## 💾 Saving Your Data
📁 Confirmed bookings and cancellations are written to `orchid_bookings.txt` in the current working folder. The program loads this file at startup.

🔢 Ticket numbers start at **1001**. After loading data, the next number is based on the highest ticket number found.

📦 The array holds up to **20,000 seat records**, including inactive records. One ticket with three seats uses three records. Cancellation does not free record storage.

⚠️ Saving rewrites the file directly. There is no temporary-file backup or rollback. A save error can leave the file and the in-memory records different. The loader also does not fully validate damaged file contents. Keep a separate copy of important data.

## 🚀 Build and Run
🧰 You need a C compiler such as GCC and a terminal. VS Code needs a compiler installed separately.

📥 **Clone and open the project**
```bash
git clone https://github.com/SyntaxMaruf/DIU-Bus-Seat-Booking-System.git
cd DIU-Bus-Seat-Booking-System
```

🔨 **Compile**
```bash
gcc -std=c99 -Wall -Wextra -pedantic bus_booking_system.c -o bus_booking_system
```

▶️ **Windows PowerShell**
```powershell
.\bus_booking_system.exe
```

▶️ **Windows Command Prompt**
```bat
bus_booking_system.exe
```

▶️ **Linux or macOS**
```bash
./bus_booking_system
```

📂 Use the same working folder each time to load the same saved records.

## 🎨 Bus Animation and Portfolio
🚌 The GIF at the top plays on the repository page. Open [bus-animation.html](bus-animation.html) locally for the larger HTML and CSS route scene.

🌐 [Visit my portfolio](https://syntaxmaruf-portfolio.curlyriver4.chatgpt.site/) to explore my work.

## 🔎 Current Limits and Future Ideas
🧪 This is a local terminal demo. It has no account login, shared database, live tracking, or online payment.

⌨️ Menu numbers and travel dates have basic checks. Name input only checks that it is not empty. Very long input and unusual data files need stronger handling.

💺 If a user asks for more seats than are available for the chosen category and route, seat selection can keep asking without a way to finish that request. A free-seat count and a back option would help.

🕒 Time handling uses minute values within a day. Trips crossing midnight need more complete date-and-time logic.

🛠️ Better input checks, safer saving, a search option, mixed passenger groups, and a database are future improvements. They are not included features.

## 🙌 Support
⭐ If you find this project useful, feel free to give it a star.

## 📜 License
📌 No license has been selected for this repository.

👨‍💻 **Maruf Billah · CSE · Daffodil International University**
