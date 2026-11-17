# Magnetic Loop Antenna Build Guide

### Indoor / Balcony HF Antenna — 40m–10m — Motorized Tuning (LDO NEMA17 0.9° \+ TMC2208)

#### Capacitor: Polaron KP1-8 · 4–250 pF · 5 kV · 8 mm shaft 
#### Motor: [LDO-42STH48-1684MAC](LDO-42STH48-1684MAC%20RevA.pdf) (Nema 17 Stepper)
#### TMC2208 Driver [Specification](TMC2208-V3.pdf)
#### Arduino Nano [Pinout](https://devboards.info/boards/arduino-nano)

---

## Table of Contents

1. [Theory & Background](#bookmark=id.4nvrjzyngjkf)  
2. [Design Calculators](#bookmark=id.cp1omqr635f4)  
3. [Bill of Materials](#bookmark=id.flb7dp3wh19y)  
4. [Assembly: The Loop](#bookmark=id.9p63si3g6nqy)  
5. [Assembly: High-Voltage Capacitor](#bookmark=id.kmv6gg6i7mxm)  
6. [Assembly: Motorized Tuning Box](#bookmark=id.vhoqrc3hb3mi)  
7. [Assembly: Coupling Loop & Feed](#bookmark=id.n8zpw931res7)  
8. [Electronics & Firmware](#bookmark=id.z3n7j77reebt)  
9. [Cable Management & Enclosure](#bookmark=id.gcqjlbmv9h7w)  
10. [Tuning & Operation](#bookmark=id.rq4kc3sn98x)  
11. [Safety Notes](#bookmark=id.fi9ohyih4a8w)

---

## 1\. Theory & Background

### What is a Magnetic Loop Antenna?

A magnetic loop (magloop) is a small transmitting loop antenna — typically less than 1/10 wavelength in circumference. It consists of:

- **Main loop**: A single-turn inductor made of thick copper or aluminium conductor  
- **Tuning capacitor**: A high-voltage variable capacitor that resonates the loop at the desired frequency  
- **Coupling loop**: A smaller loop (1/5 the diameter of the main loop) that inductively couples your feedline to the antenna

The loop \+ capacitor form a high-Q parallel resonant circuit. Because of the extremely high Q (typically 500–1000+), the antenna is very narrowband (10–30 kHz bandwidth), meaning:

- Excellent selectivity and low noise on receive  
- Requires retuning even for small frequency changes (hence motorized tuning)  
- Very high voltages develop across the capacitor (several kV at 100W)

### Key Electrical Parameters

This build uses a **1.2m diameter circular loop** made from **22mm copper pipe**. The 1.2m diameter was chosen primarily because it **boosts 20m efficiency from \~60% to \~78%** compared to a 1.0m loop — a real \~2 dB gain on the most popular HF band. As a bonus, the larger loop also makes 40m usable for digital modes and CW, though with significant limitations (see below).

**Use the 66pacific calculator (Section 2\) with your exact dimensions to verify before purchasing components.**

#### 20m Band — 14.175 MHz @ 100W

| Parameter | Value |
| :---- | :---- |
| Efficiency | \~78% (-1.1 dB) |
| Bandwidth (-3dB) | \~25 kHz |
| Tuning capacitance | \~45 pF |
| Capacitor voltage | **\~4,800 V RMS** (\~6,800V peak) |
| Circulating current | \~20 A |
| Radiation resistance | 0.199 Ω |
| Loss resistance | 0.045 Ω |
| Q factor | \~510 |

#### 40m Band — 7.1 MHz @ 60W

| Parameter | Value |
| :---- | :---- |
| Efficiency | \~22% (-6.6 dB) |
| Bandwidth (-3dB) | \~4–5 kHz |
| Tuning capacitance | \~175 pF |
| Capacitor voltage | **\~4,300 V RMS** (\~6,100V peak) |
| Circulating current | \~35 A |
| Radiation resistance | 0.013 Ω |
| Loss resistance | 0.038 Ω |
| Q factor | \~1,250 |

#### Power Strategy by Band — Polaron KP1-8 (5 kV rated)

The KP1-8 is rated at **5 kV working voltage (peak)**.

| Band | Peak cap voltage at given power | Safe max power(KP1-8 5 kV) |
| :---- | :---- | :---- |
| 40m (7 MHz) | \~6,100 V pk at 60W | **30W** |
| 30m (10 MHz) | \~4,800 V pk at 70W | **70W** |
| 20m (14 MHz) | \~6,800 V pk at 100W | **35W** |
| 17m (18 MHz) | \~5,200 V pk at 100W | **75W** |
| 15m–10m (21–28 MHz) | well below 5 kV | **100W** |

> **20m note:** The 20m peak voltage at 100W is \~6,800 V — above the 5 kV KP1-8 rating. Limit to **35W on 20m** to stay within the capacitor's rating with a safe margin. On 17m and above, 100W is fine. This is the trade-off for the 5 kV cap; a 7 kV replacement would allow full 100W across all bands.  
>   
> 22% efficiency on 40m sounds poor, but in practice it means about **\-6.6 dB compared to a perfect antenna** — roughly one S-unit. Many operators make successful FT8, CW, and even SSB contacts on 40m with small loops because digital modes and CW work well at lower power, and the loop's quiet receive compensates for the lower TX efficiency.

### Why 1.2m? — Design Rationale

The 1.2m diameter is a sweet spot for an indoor/balcony loop:

| Aspect | 1.0m loop | 1.2m loop | Verdict |
| :---- | :---- | :---- | :---- |
| 20m efficiency | \~60% | **\~78%** | \+2 dB — biggest real-world gain |
| 40m capability | Barely usable (\~15%) | Usable for FT8/CW (\~22%) | Nice bonus |
| 10m tuning | 18 pF (comfortable) | 10–12 pF (tight — see note below) | Minor edge case |
| 12m/10m pattern | Clean figure-8 | Slightly distorted lobes | Academic difference |
| Physical size | 1.0m wide | 1.2m wide | Fits most balconies |

> **10m edge case:** At 28 MHz, the 1.2m loop needs only \~10–12 pF to resonate. This is close to the minimum capacitance of most vacuum caps (5 pF) plus stray wiring capacitance (\~3–5 pF). You may not be able to tune to the **top** of 10m (28.5–29.7 MHz). Verify with the calculator once you know your capacitor's exact minimum. In practice, the lower portion of 10m (28.0–28.3 MHz) should still be reachable. This is unlikely to be a problem unless you specifically need the upper segments of 10m.

**Bottom line:** The 1.2m loop is the right choice. The 20m improvement alone justifies the 20% size increase, and 40m is a free bonus for digital modes. The 10m concern is a minor edge case that affects a small portion of one band.

### Why It Works Indoors

- The near-field is predominantly **magnetic**, which suffers less loss from nearby building materials than the electric field of a dipole  
- Works well even close to the ground or inside apartments  
- Deep pattern nulls allow you to reject local interference sources  
- No ground plane or counterpoise required

### Trade-offs

You can have any TWO of: **Small Size**, **Efficiency**, **Broadband**. The magloop sacrifices bandwidth for small size and efficiency. The narrower the bandwidth (higher Q), the more efficient it is — but the more often you must retune.

### Frequency Coverage

A single loop typically covers about **2 octaves** (e.g., 7–28 MHz). Coverage depends on loop size and capacitor range.

For **20m–10m** operation (primary design range), the 1.2m loop needs a capacitor range of **\~10–50 pF**. For **40m as a bonus**, you need the capacitor to reach **\~175 pF**. The **Polaron KP1-8 (4–250 pF)** covers the full range from 40m down to 10m with margin — no need for a 500 pF cap with a 1.2m loop.

> **10m minimum capacitance check:** The KP1-8's 4 pF minimum plus \~3–5 pF of stray wiring capacitance gives an effective minimum of \~7–9 pF. Resonating the 1.2m loop at 28.0–28.3 MHz needs \~10–12 pF, which is borderline. You may not reach the upper part of 10m (28.5–29.7 MHz). Verify with the calculator and your specific stray capacitance after assembly.

On 17m–10m, 100W is safe within the KP1-8's rating. On 20m, limit to **35W** (peak cap voltage \~6,800 V at 100W — exceeds the 5 kV rating). On 40m, limit to **30W** (see Power Strategy table).

---

## 2\. Design Calculators

Use these tools to determine exact dimensions, capacitor requirements, and voltage ratings for your specific build:

| Calculator | URL | Notes |
| :---- | :---- | :---- |
| 66pacific Loop Calculator | [http\://www\.66pacific.com/calculators/small-transmitting-loop-antenna-calculator.aspx](http://www.66pacific.com/calculators/small-transmitting-loop-antenna-calculator.aspx) | Best online calculator — enter loop size, frequency, power → get cap voltage, efficiency, bandwidth |
| AA5TB Excel Spreadsheet | [https\://aa5tb.com/aa5tb\_loop\_v1.22f.xlsx](https://aa5tb.com/aa5tb_loop_v1.22f.xlsx) | Downloadable Excel tool, very detailed |
| G4FGQ RJELOOP1 | [https\://m0ukd.com/homebrew/antennas/magnetic-loop-antennas/](https://m0ukd.com/homebrew/antennas/magnetic-loop-antennas/) | DOS executable, runs in DOSBox |

**Recommended workflow:**

1. Open the 66pacific calculator  
2. Set conductor diameter \= 22mm (for 22mm copper pipe)  
3. Set loop circumference \= **3.77m** (1.2m diameter circle)  
4. **Run it twice:** once at **7.1 MHz / 60W** (40m worst-case) and once at **14.175 MHz / 100W** (20m)  
5. Note the **capacitor voltage** at each band — the highest value determines your minimum capacitor voltage rating  
6. Note the **tuning capacitance** at the lowest frequency — this determines the maximum capacitance your variable cap must reach

---

##  3\. Bill of Materials

> **Sourcing notes for Romania:** Primary links are to AliExpress (free/cheap shipping to RO), eBay (for vacuum caps), Amazon.de (ships to RO via EU), and Romanian shops like Optimus Digital (optimusdigital.ro) and Robofun (robofun.ro). For copper pipe and hardware: Dedeman, Leroy Merlin, or local plumbing suppliers.

### 3.1 Main Loop

| Item | Specification | Qty | Source | Est. Price |
| :---- | :---- | :---- | :---- | :---- |
| Copper pipe · *Țeavă de cupru* | 22mm OD (⅞"), **3.8m length** (for 1.2m Ø loop), soft or hard | 1  | Dedeman / Leroy Merlin | \~60–100 RON |
| Copper pipe end caps · *Capace (dopuri) țeavă cupru* | 22mm | 2 | Same as above | \~5 RON |
| SO-239 chassis mount connector  | UHF female, panel mount | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-so239-chassis-mount.html) | \~8 RON |
| Copper braid / strap · *Bandă (împletitură) de cupru* | 15mm wide, 10cm length (for cap connection) | 2 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-copper-braid-strap.html) | \~10 RON |
| Stainless steel hose clamps · *Coliere inox pentru furtun* | 22mm–32mm range | 4 ✔️ | Dedeman | \~10 RON |

### 3.2 Tuning Capacitor

**Polaron KP1-8 — Vacuum Variable Capacitor (confirmed)**

| Item | Specification | Qty | Source | Est. Price |
| :---- | :---- | :---- | :---- | :---- |
| Polaron KP1-8 vacuum variable capacitor | **4–250 pF**, **5 kV** working voltage, 8 mm output shaft, 22.5 turns full travel \- MEASURED | 1 ✔️ | eBay / surplus dealers | €150-200 |

> **Capacitance range check for this build:** At 7.1 MHz (40m) with a 1.2m loop, you need \~175 pF — within the 250 pF maximum. At 14.175 MHz (20m), \~45 pF — well within range. At 28 MHz (10m), \~10–12 pF — close to the effective minimum (4 pF \+ \~5 pF stray ≈ 9 pF). The KP1-8 covers 40m through 10m comfortably for this loop.  
>   
> **Handling:** Vacuum capacitors are fragile. Never drop, strike, or force rotation past the mechanical stops. The vacuum seal, if broken, cannot be repaired.

### 3.3 Motorized Tuning System

| Item | Specification | Qty | Source | Est. Price |
| :---- | :---- | :---- | :---- | :---- |
| NEMA17 stepper motor | **LDO-42STH48-1684MAC**, 0.9°/step (400 steps/rev), 1.68A, 48mm body | 1 ✔️ | [LDO Motors distributors](https://ldomotors.com) / AliExpress | \~45–70 RON |
| TMC2208 stepper driver | Silent driver module, UART capable, with heatsink | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-TMC2208-stepper-driver.html) / [Optimus Digital](https://www.optimusdigital.ro) | \~15–25 RON |
| Arduino Nano | ATmega328P, USB Mini/Micro | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-arduino-nano-v3.html) / [Optimus Digital](https://www.optimusdigital.ro) | \~15–25 RON |
| Beam coupling 5mm → 8mm | Aluminium single-piece slit coupling, 5 mm bore × 10 mm bore (motor shaft → KP1-8 shaft), e.g. D19L25 | 1 ✔️ | AliExpress: search “beam coupling 5mm 8mm” | \~8–12 RON |
| Metal planetary gearbox *(optional)* | NEMA17 compatible, **5:1 ratio**, all-metal gears, output shaft 8mm | 1 | AliExpress | \~60–90 RON |
| Miniature microswitches (end stops) | Omron D2F-L or equivalent, snap-action, NC | 2 ✔️ | AliExpress / Optimus Digital | \~5 RON |
| Commercial NEMA17 aluminium motor bracket | L-shaped aluminium, pre-drilled NEMA17 pattern | 1 ✔️ | AliExpress: “NEMA17 motor mount bracket” | \~8–12 RON |
| 100µF/50V electrolytic capacitor | For motor driver power filtering | 1 ✔️ | Optimus Digital / AliExpress | \~2 RON |
| Rotary encoder | KY-040 or similar, with push button | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-rotary-encoder-ky040.html) | \~5 RON |
| OLED display | 0.96” I2C SSD1306, 128×64 | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-oled-0.96-i2c.html) | \~12 RON |
| HC-05 / HC-06 Bluetooth module *(optional)* | Serial TTL, for wireless control | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-hc-05-bluetooth-module.html) | \~15 RON |
| DC barrel jack | 5.5×2.1mm, panel mount | 1 ✔️ | AliExpress / Optimus Digital | \~3 RON |
| Power supply | 12V 2A DC adapter | 1 ✔️ | AliExpress / Dedeman | \~25 RON |
| Perfboard / PCB | 5×7 cm or 7×9 cm | 1 ✔️ | AliExpress / Optimus Digital | \~5 RON |

> **Beam coupling vs jaw coupling:** Both are available in 5mm→8mm. The beam coupling (single aluminium piece) is preferred — no rubber to fatigue, handles slight angular misalignment, zero backlash for the torque levels involved.

### 3.4 Enclosure & Hardware

| Item | Specification | Qty | Source | Est. Price |
| :---- | :---- | :---- | :---- | :---- |
| Weatherproof junction box (top / capacitor box) | ABS IP65, **220×150×80mm minimum** (motor \+ coupling \+ KP1-8 require \~200mm along shaft axis) | 1 ✔️ | Dedeman / [AliExpress](https://www.aliexpress.com/w/wholesale-waterproof-junction-box-200x120.html) | \~30–50 RON |
| Indoor control box | ABS project box, 120×80×40mm (Arduino \+ TMC2208 \+ display \+ encoder) | 1 ✔️ | AliExpress / Optimus Digital | \~15 RON |
| PVC pipe · *Țeavă PVC* | 32mm or 40mm, 1.5m (antenna support mast) | 1 | Dedeman | \~15 RON |
| PVC T-junction · *Teu PVC* | Matching pipe diameter | 1 | Dedeman | \~5 RON |
| Cable glands · *Presetupe* | PG7 and PG9 | 3 ✔️ | Dedeman / AliExpress | \~8 RON |
| RG-58 coaxial cable | 50Ω, 3–5 meters | 1 ✔️ | [AliExpress](https://www.aliexpress.com/w/wholesale-rg58-coaxial-cable.html)  | \~25 RON |
| PL-259 UHF connector | Solder type for RG-58 | 2 ✔️ | AliExpress | \~8 RON |
| 4-wire shielded cable · *Cablu ecranat cu 4 fire* | For stepper motor, 3–5m (cat5 works) | 1 ✔️ | Local shop / AliExpress | \~15 RON |
| Heat shrink tubing · *Tub termocontractibil* | Assorted sizes | 1 set ✔️ | AliExpress | \~8 RON |
| Self-amalgamating tape · *Bandă autovulcanizantă* | For weatherproofing outdoor joints | 1 roll✔️ | Dedeman / AliExpress | \~15 RON |
| Zip ties · *Coliere de plastic* | Various sizes |  ✔️ | Dedeman | \~8 RON |
| M3 and M4 bolts \+ nuts · *Șuruburi și piulițe M3/M4, inox* | Stainless steel assortment | 1 set | Dedeman | \~15 RON |
| Aluminium flat plate | 3–5mm thick, \~100×80mm (for capacitor collar mount) | 1 ✔️ | Metal offcut / Dedeman | \~10 RON |

### 3.5 Coupling Loop

\| Item \| Specification \| Qty \| Source \| Est. Price \| \|---\|---\|---\|---\| \| RG-213 or RG-58 coax · *Cablu coaxial RG-213 sau RG-58* \| 50Ω, **\~75 cm** (1/5 of main loop circumference) \| 1 \| Leftover from feedline / local shop \| \~10 RON \|

### Estimated Total Cost

| Category | Estimated Cost |
| :---- | :---- |
| Loop materials (copper pipe, connectors) | \~90–120 RON |
| Polaron KP1-8 vacuum variable capacitor | \~150–500 RON (€30–100) |
| LDO motor \+ TMC2208 \+ Arduino \+ optional gearbox | \~100–185 RON |
| Enclosure \+ hardware (metal brackets, switches) | \~90–130 RON |
| Cables \+ connectors | \~60–80 RON |
| **Total** | **\~490–1,000 RON (\~€100–200)** |

---

## 4\. Assembly: The Loop

### 4.1 Forming the Loop

1. **Straighten the copper pipe** — If using soft copper in a coil, carefully straighten it on a flat surface. Avoid kinks; they increase resistance and create stress points.  
     
2. **Form the circle** — For a **1.2m diameter** circle, you need π × 1.2m ≈ **3.77m** of pipe. Buy 3.8m to have a small margin. Bend the pipe gently and gradually into a circle. Use a large round object (barrel, wheel rim, tree) as a form if available. For hard copper pipe, use a pipe bender or fill with sand, cap both ends, and bend slowly.  
     
3. **Leave a gap at the top** — The two ends of the pipe should end up at the top, with a gap of **3–5 cm** between them. This is where the tuning capacitor connects.  
     
4. **Flatten the ends** (optional) — Lightly flatten the last 2 cm of each pipe end to create a surface for bolting the capacitor connections. Drill a hole through each flattened end for M4 bolts.

### 4.2 Mounting the Loop

5. **Attach to the support mast** — At the bottom of the loop (directly opposite the capacitor gap), secure the pipe to a PVC mast using:  
     
   - Two stainless hose clamps around the copper pipe and PVC pipe, or  
   - A commercial aluminium pipe clamp (saddle clamp) sized for 22mm pipe  
   - The loop should be mounted **vertically** for normal operation

   

6. **Ensure the capacitor gap is at the top** — This keeps the high-voltage point away from people and reduces capacitive coupling to ground.

         ┌──── GAP ────┐

         │   \[CAPACITOR\]  │

         │   \[  MOTOR  \]  │

        ╱                  ╲

       │                    │

       │    MAIN LOOP       │

       │    (copper pipe)   │

        ╲                  ╱

         ╲                ╱

          └──\[CLAMP\]─────┘

              │

         \[PVC MAST\]

              │

         \[COUPLING LOOP\]

         (at the bottom)

### 4.3 Enclosed Balcony Mounting (4m × 1m × 2m)

This section covers placement for a typical closed/enclosed balcony (glass walls, \~4m long × 1m wide × 2m high).

#### Placement Strategy

 ══════════ OUTER WALL (glass/window side) ═══════════

 │                                                    │

 │            ┌───────────┐                           │

 │            │ \[CAP+MOT\] │ ← 1.6m height            │

 │           ╱             ╲                          │

 │          │    1.2m Ø     │ ← center at \~1.0m       │ 2m

 │          │     LOOP      │                         │ high

 │           ╲             ╱                          │

 │            └─────┬─────┘  ← bottom at \~0.4m       │

 │            \[PVC MAST\]                              │

 │         \[COUPLING LOOP\]                            │

 │          \[CONTROL BOX\]   ← on floor / low shelf   │

 │                                                    │

 ═══════════ INNER WALL (apartment side) ═════════════

                   1m wide

 ├────────────────────────────────────────────────────┤

                   4m long

#### Key Rules

1. **Orient the loop plane parallel to the long wall (4m side)** — The loop's radiation maxima are in the plane of the loop, so this aims your signal outward through the glass on both sides. The nulls point at the short walls (1m sides).  
     
2. **Position as close to the glass/outer wall as possible** — Even 10–20 cm closer to the glass side helps. The glass is relatively RF-transparent; concrete and metal frames are not.  
     
3. **Keep ≥30 cm from metal window frames and reinforced concrete** — Metal in the near-field causes eddy current losses that reduce efficiency. Aluminium window frames are the biggest offender. If your frame is aluminium, try to center the loop between the frames rather than right next to one.  
     
4. **The loop only uses 22mm of depth** — The 1.2m diameter loop mounted vertically takes practically zero width. It will sit flat against the wall side with just the PVC mast and control box taking floor space.  
     
5. **Height positioning:**  
     
   - Loop center at **\~1.0m** (middle of your 2m space)  
   - Capacitor box at the top: **\~1.6m** — above casual reach but accessible for maintenance  
   - Loop bottom: **\~0.4m** above floor  
   - Coupling loop \+ feedline exit at the bottom

   

6. **Mast options for enclosed balcony:**  
     
   - **Floor-standing PVC mast** with a weighted base (bucket with sand/concrete, or a Christmas tree stand)  
   - **Wall-mounted brackets** — two PVC pipe clips screwed to the wall, loop slides in/out for storage  
   - **Ceiling-suspended** — hang from a hook with nylon cord if you want to keep the floor clear

#### Performance in Enclosed Balcony

Expect the following compared to free-air outdoor mounting:

- **1–3 dB additional loss** from nearby walls, glass, and metal frames  
- **SWR may be slightly harder to optimize** — the coupling loop may need more adjustment  
- **The loop still works well** — magnetic near-field is less affected by surroundings than electric. Many operators report surprisingly good results from enclosed balconies  
- **Rotating the loop** (even by hand) to null interference is easy in this setup — just turn the mast

>   
> **Pro tip:** If your enclosed balcony has an openable window or section, position the loop so it radiates through the opening when it's open in good weather. Close it up when not operating — the loop still works through glass, just slightly less efficiently.

#### Safety in Enclosed Space

⚠️ The high-voltage capacitor (top of loop) is at \~1.6m — **within arm's reach**.

- **Never transmit while anyone can touch the top of the loop**  
- Consider adding a **clear acrylic shield** or **"DO NOT TOUCH — HIGH VOLTAGE" warning label** near the capacitor box  
- If children or pets have access to the balcony, add a physical barrier or only operate when the balcony door is closed  
- Reduce power for casual indoor/balcony operation — **25W is a reasonable compromise** for safety and still effective for FT8/CW on most bands

---

## 5\. Assembly: High-Voltage Capacitor

### 5.1 Vacuum Variable Capacitor

If using a vacuum variable capacitor:

1. **Inspect the capacitor** — Check for physical damage. The vacuum seal must be intact. If the internal parts are loose or rattling, the cap is likely broken.  
     
2. **Verify the shaft rotation** — Turn the adjustment shaft gently. It should rotate smoothly through the full range (22.5 turns measured).  
     
3. **Connect to the loop** — The capacitor has two terminals (stator plates). Connect each terminal to one end of the copper loop using:  
     
   - Short, wide copper straps (≥15mm wide) or copper braid  
   - Bolt connections with stainless M4 hardware  
   - **Keep the connections as short as possible** — every mm of extra wire adds loss  
   - Solder or bolt firmly; loose connections \= resistive losses \= heat

   

4. **Orientation** — Mount the capacitor with its adjustment shaft accessible for the stepper motor coupling.

### 5.2 Rotary Shaft Mounting — KP1-8 Specifics

The Polaron KP1-8 is a rotary vacuum variable capacitor with the following mechanical characteristics relevant to the drive system:

- **Output shaft diameter: 8 mm** (measured)   
  \[Safety – CRITICAL\] Before coupling the motor: measure with an ohmmeter between the shaft and each brass band (the two KP1-8 terminals). The shaft comes out of a plastic drive head, so it may be isolated — but if it reads continuity to the upper band, it is at several kV RF while transmitting. The printed LINK's 30 mm insulator section covers both cases (see 6.2.5 comment); never replace it with a metal coupling.   
- **Full travel: 22.5 turns** from minimum (4 pF) to maximum (250 pF) capacitance  
- **Replacement mounting**: two ring clamps (copper/brass band clamps, Ø70) around the brass bands, each on an insulating post (ASA/PTFE, nylon M4 screws) on the enclosure floor; the loop straps bolt to the ring clamps. Never clamp or load the glass. Shaft axis ≈ 50 mm above the floor (radius 35 \+ 15). See magloop\_mounting\_schema.png.  
- **Rotation torque:** Very light — a finger can turn the shaft — so motor and gearbox torque requirements are trivially low

#### Mounting Bracket (metal only — no 3D printing)

1. **Cut a capacitor plate** from 3–5mm aluminium flat stock (\~100×80mm).  
2. **Tighten the collar locknut** against the rear face of the plate to secure the capacitor rigidly.  
3. **Bolt the plate to the enclosure wall** using M4 bolts — two bolts minimum, top and bottom.

#### Motor–Capacitor Shaft Coupling

The **LDO-42STH48-1684MAC motor shaft is 5 mm**. The **KP1-8 shaft is 10 mm**. You must bridge this mismatch with an appropriate coupling:

| Scenario | Coupling required | Notes |
| :---- | :---- | :---- |
| **Direct drive (no gearbox)** | Beam coupling, 5mm × 8mm bore | Single aluminium piece, zero backlash, handles ±1° angular misalignment |
| **With 5:1 gearbox (8mm output)** | Beam or jaw coupling, 8mm × 10mm bore | Verify gearbox output shaft diameter before ordering |
| **With 5:1 gearbox (10mm output)** | Rigid or flexible coupling, 10mm × 10mm bore | Rigid coupling acceptable only if shafts are co-axial |

> **Recommended: beam coupling 5mm × 8mm** (direct drive D19L25). Set the 8mm-side set screw on the flat of the KP1-8 shaft — file a flat if the shaft is round, to prevent rotation slip.  
> **\[Safety\]**: if the shaft is electrically live, this coupling must be an insulated one (fibreglass/PTFE extension), not a direct metal beam coupling. 

#### End Stops — Mechanical Protection

⚠️ Driving the motor past the KP1-8's physical end stops **will damage the capacitor**. Protect against this with:

1. **Software limits** (mandatory minimum): set `MAX_POSITION` conservatively in firmware, established during initial calibration (see Section 8).  
2. **Hardware microswitches** (strongly recommended): two miniature snap-action switches (Omron D2F-L or equivalent) mounted on an aluminium strip inside the enclosure. One switch at each end of the capacitor's travel range, wired to Arduino digital inputs (INPUT\_PULLUP, NC contact).  
   *Firmware behaviour*: on a switch trip it stops immediately, raises FAULT and locks out motion until 'home' is sent (it does not 'reverse'). A trip during normal use means lost steps or a broken wire, so position is no longer trusted. 

Align the microswitches so they trip **0.5–1 turns before** the mechanical limit as a safety buffer.

---

## 6\. Assembly: Motorized Tuning Box

This is the heart of the automatic tuning system. The LDO-42STH48-1684MAC stepper motor drives the Polaron KP1-8 vacuum capacitor via a flexible coupling, controlled by a TMC2208 driver and an Arduino Nano. No 3D-printed parts are used anywhere in this build.

---

### 6.0 Drive System — Resolution Analysis

Before mechanical assembly, understand what you are working with:

| Parameter | Value |
| :---- | :---- |
| Motor step angle | 0.9°/step (LDO-42STH48-1684MAC) |
| Steps per revolution | **400 steps/rev** |
| TMC2208 microstepping | 256 µstep (default, MS pins floating) |
| Microsteps per motor revolution | **400 × 256 \= 102,400 µstep/rev** |
| KP1-8 total travel | 22.5 full turns (4 pF → 250 pF) |
| Total microsteps (no gearbox) | **22.5 × 102,400 \= 2,304,000 µstep** |
| Total microsteps (5:1 gearbox) | **22.5 × 5 × 102,400 \= 11,520,000 µstep** |

**Practical implication:** On 40m, the \~4–5 kHz bandwidth corresponds to a capacitance change of roughly 2–3 pF (\~1% of the KP1-8's range). Without any gearbox, that 1% of travel spans **\~720 microsteps** — plenty. **A gearbox is optional.** Add one only if you want extra mechanical torque margin or find the coupling mechanically convenient.

---

### 6.1 Gearbox Decision

| Option | Pros | Cons | Recommended for |
| :---- | :---- | :---- | :---- |
| **No gearbox — direct drive** | Simplest; fewest parts; fastest tuning sweep | None significant at these torque levels | **This build — preferred** |
| **5:1 metal planetary gearbox** | 5× torque; 5× finer effective resolution | \~35mm extra length; gearbox must fit in enclosure; 8mm output needs 8mm→10mm coupling | If you want extra mechanical robustness |

---

### 6.2 Mechanical Assembly — Step by Step       

#### 6.2.1 Capacitor Plate (Aluminium)

The KP1-8 is held in place by its threaded collar/flange clamping through a plate:

1. Obtain a piece of **3–5mm aluminium flat plate**, approximately **100×80mm**.  
2. Ring clamps on the two brass bands   
3. Pass the capacitor through the hole from the front face. Thread the collar locknut from the rear and tighten — this clamps the capacitor rigidly to the plate.  
4. Drill four **M4 holes** near the corners to bolt the plate to the enclosure rear wall.  
5. Ensure the KP1-8 **adjustment shaft protrudes into the enclosure** (toward the motor).

  \[Enclosure rear wall\]  ← bolt through here

         │

  \[Aluminium plate\]      ← 3–5mm thick, M4 bolted to wall

         │

  \[KP1-8 collar nut\]    ← tightened against rear of plate

         │

  \[KP1-8 body\]          ← passes through hole in plate

         │

  \[8mm shaft\]          ← protrudes inward toward motor ──► coupling

#### 6.2.2 Motor Bracket — Option A: Commercial NEMA17 L-Bracket

The simplest approach. Available on AliExpress as *"NEMA17 motor mount bracket"* — a pre-drilled aluminium L-extrusion with the 42mm NEMA17 bolt pattern and central shaft clearance hole already machined.

1. Bolt the motor to the bracket with **4× M3×8mm** screws (M3 is NEMA17 standard).  
2. Bolt the bracket foot to the **enclosure floor** using **M4 countersunk screws** through the enclosure base.  
3. Adjust the bracket position (slot the holes slightly if needed) so the motor shaft is **co-axial with the KP1-8 shaft** before final tightening.

#### 6.2.3 Motor Bracket — Option B: DIY Aluminium Angle

If you prefer to fabricate from raw stock (aluminium from Dedeman):

**Materials:** 40×40×3mm aluminium L-angle, 120mm length

1. On the **vertical face** of the angle: mark and drill 4× M3 holes in the **NEMA17 31.04mm bolt circle** pattern, centred, plus a **22mm central clearance hole** for the motor shaft.  
2. On the **horizontal face** (base): drill 2× M4 holes for bolting to the enclosure floor.  
3. Deburr all holes. Bolt motor to vertical face; bolt angle to enclosure floor.

  Top view:

  ┌──────────────┐ ← Motor face (vertical)

  │  M3 ·   · M3│   4× M3 bolts, 31mm pattern

  │    · SHAFT · │   22mm shaft clearance hole

  │  M3 ·   · M3│

  └──────┬───────┘

         │ aluminium angle

  ═══════╪═══════════ ← Enclosure floor

         │ M4 bolts

#### 6.2.4 Shaft Coupling Installation

**Direct drive (5mm motor → 8mm capacitor):** beam coupling D19L25 (or similar)

1. Slide the **8mm bore end** of the beam coupling onto the KP1-8 shaft about 15mm.  
2. Before tightening: confirm the shaft has a flat (key). If the KP1-8 shaft is fully round, **file a small flat** \~0.5mm deep with a needle file — this prevents rotational slip under the set screw.  
3. Do **not** tighten the set screw yet.  
4. Offer the motor bracket (with motor) toward the capacitor plate, inserting the **5mm motor shaft** into the 5mm bore end of the coupling.  
5. Adjust bracket position until the coupling sits centred on both shafts with no lateral force.  
6. Tighten bracket bolts first, then tighten both coupling set screws (Allen key).  
7. **Test:** rotate by hand through the full 22.5 turn range — zero binding, no wobble.

**If using a 5:1 gearbox:**

- Bolt gearbox to motor first (NEMA17 face plate, 4× M3)  
- Use an **8mm→10mm beam coupling** to connect the gearbox output to the KP1-8 shaft  
  \[Firmware\] If you fit the gearbox, set GEAR\_RATIO \= 5 in the firmware; steps/turn, MAX\_POSITION, coarse step, back-off and default presets scale automatically. Sweep and homing become 5× slower (\~100 s full sweep, homing up to \~8 min). Re-save all band presets after changing it.   
- The motor bracket must accommodate the extra \~35mm of gearbox length (use a longer aluminium angle or mount the gearbox directly to the enclosure wall with M3 standoffs)

#### 6.2.5 End-Stop Switches (Strongly Recommended)

The KP1-8 shaft does not move axially, so a tab on the coupling would hit a switch once per turn, not at the ends of travel.  
**End-Stop Switches — Travelling Nut**. The coupling only rotates, so the 22.5 turns are converted into linear travel by a travelling nut on the shaft line. A 3D-printed LINK part (PETG, 100 % infill) replaces the direct shaft connection: \[8 mm clamp \+ M3 set screw on the KP1-8 shaft flat\] → \[30 mm plain insulator\] → \[50 mm printed thread, 12 mm × 1.5 mm pitch\] → \[8 mm stub into the 5×8 beam coupling\]. A printed NUT with a downward fin rides on the thread; the fin slides in the slot of a printed RAIL bolted to the enclosure floor, so the nut cannot rotate and moves 1.5 mm per shaft turn (33.75 mm over the full range). Two D2F-L switches lie flat at the ends of the rail, levers facing the fin, in slotted M2 holes (±4 mm adjustment). Files: magloop\_endstop.scad (parametric) and magloop\_endstop\_link/nut/rail.stl.'  
The insulator section also isolates the motor if the KP1-8 shaft turns out to be at RF potential 

  \[MIN switch\] ← fin ←NUT on printed thread → fin → \[MAX switch\]

Nut travel between trip points \= (22.5 − 2 × 0.75) turns × 1.5 mm \= 31.5 mm. 

**Wiring:** Matches the firmware (D9/D10, INPUT\_PULLUP, NC to GND, HIGH \= tripped; a broken wire also reads as tripped, which is fail-safe). Switch lines run 3–5 m through a strong RF field: add 1 kΩ in series \+ 100 nF to GND at each Nano input, use shielded cable, and a ferrite at the box entry. Firmware already ignores spikes \< \~40 µs. Firmware stops and faults — it does not reverse. 

**Assembly & adjustment:**   
(1) Clamp the LINK on the KP1-8 shaft (set screw on the filed flat \+ clamp bolt; it must never slip — a slip moves the switches relative to the capacitor's stops).   
(2) Turn the capacitor BY HAND to its minimum-capacitance stop.   
(3) Thread the nut from the stub end until it is \~2 mm from the insulator shoulder, fin pointing down.   
(4) Fit the beam coupling \+ motor.   
(5) Bolt the rail so the fin sits in the slot.   
(6) Slide the MIN switch until it just clicks, then back it off \~1.1 mm (0.75 turn) away from the nut and tighten.   
(7) Turn the shaft by hand to the maximum stop and repeat for MAX.   
(8) Check by hand over the full range: smooth, nut never binds, both switches click before each stop. 

---

### 6.3 Enclosure Layout

Use a **\~380×190×130 mm (minimum) IP65 ABS enclosure**. This comfortably accommodates:

- LDO NEMA17 (48mm body \+ 22mm shaft \= 70mm)  
- Optional gearbox (\~35mm)  
- D19L25 beam coupling (\~25mm)  
- KP1-8 body depth from plate (\~50mm)

**Recommended: split layout** — motor \+ capacitor in the weatherproof top box; Arduino \+ TMC2208 \+ display \+ encoder in a **separate small indoor control box** (120×80×40mm). This keeps electronics accessible and dry, and makes the top box as small as possible.

  TOP BOX (IP65, at the loop capacitor gap)

  ┌────────────────────────────────────────────────────────┐

  │                                                        │

  │  \[NEMA17\]──\[Gearbox opt.\]──\[Coupling\]──\[KP1-8\]──► loop│

  │  \[motor bracket\]           \[cap plate\]                │

  │                                                        │

  │  \[Switch MIN\]                          \[Switch MAX\]   │

  │                                                        │

  │  \[DC gland\]  \[5-wire cable gland\]  \[2× coax glands\]  │

  └────────────────────────────────────────────────────────┘

**7 or 8-wire cable**  
With the TMC2208 indoors (as drawn): 4 motor wires \+ MIN \+ MAX \+ switch GND \= 7\.   
With the TMC2208 in the top box: STEP, DIR, EN, VIO (5 V), GND, 12 V, MIN, MAX \= 8\.   
Use shielded Cat5e/Cat6 S/FTP with the TMC2208 indoors: orange pair \= coil A (1A/1B), green \= coil B (2A/2B), blue \= MIN/GND, brown \= MAX/GND, shield to GND at the control box only. Single 24 AWG wires are fine at 0.6 A over ≤5 m. 

  INDOOR CONTROL BOX (at base of mast or inside shack)

  ┌──────────────────────────────────────┐

  │  \[Arduino Nano\]  \[TMC2208+heatsink\] │

  │  \[OLED 0.96"\]    \[KY-040 encoder\]  │

  │  \[DC jack 12V\]   \[100µF filter cap\] │

  └──────────────────────────────────────┘

> If you prefer an all-in-one top box, use a **250×150×100mm** enclosure. Run the encoder and OLED cables through a cable gland to a small panel on the side of the box, within reach for tuning.

---

### 6.4 Wiring the Electronics

**TMC2208 connections:**

| TMC2208 Pin | Connect to | Notes |
| :---- | :---- | :---- |
| VCC / VMOT | 12V supply (+) |  |
| VIO / VDD | Nano 5V | note the 10 kΩ EN pull-up\! |
| GND | 12V supply (−) and Arduino GND | Common ground |
| EN | Arduino D8 | LOW \= enabled; HIGH \= coils de-energised |
| STEP | Arduino D7 |  |
| DIR | Arduino D4 |  |
| MS1 | Leave floating | TMC2208 Floating \= 1/8 stepTie to GND for noise immunity   |
| MS2 | Leave floating |  |
| 1A, 1B | LDO motor coil A | Red and Blue wires (verify with continuity tester) |
| 2A, 2B | LDO motor coil B | Green and Black wires |

> **LDO-42STH48-1684MAC coil identification:** Use a multimeter in continuity/resistance mode. Each coil pair shows \~0.9–1.1Ω between its two wires. Typical LDO colour code: coil A \= **red/blue**, coil B \= **green/black**. If the motor judders or overheats, swap one pair.

**Full wiring diagram:**

                    12V DC

                      │

              ┌───────┴───────┐

              │  100µF / 50V  │

              │  (filter cap) │

              └───────┬───────┘

                      │

           ┌──────────┴──────────┐

           │       TMC2208        │

           │  VMOT ←── 12V        │

           │  GND  ←── GND        │

           │  EN   ←── D8         │

           │  STEP ←── D7         │

           │  DIR  ←── D4         │

           │  MS1  ──  (float)    │  ← Floating MS1/MS2 \= 1/8 step 

           │  MS2  ──  (float)    │

           │  1A ──→ Motor Red    │

           │  1B ──→ Motor Blue   │  ← LDO coil A

           │  2A ──→ Motor Green  │

           │  2B ──→ Motor Black  │  ← LDO coil B

           └─────────────────────┘

           ┌─────────────────────┐

           │    Arduino Nano      │

           │  D7  → STEP          │

           │  D4  → DIR           │

           │  D8  → EN            │

           │  D2  → Encoder CLK   │

           │  D3  → Encoder DT    │

           │  D6  → Encoder SW    │

           │  D9  → MIN switch    │  ← INPUT\_PULLUP, NC contact to GND

           │  D10 → MAX switch    │  ← INPUT\_PULLUP, NC contact to GND

           │  D11 → HC-05 TX       │

           │  D12 → HC-05 RX       │ ← via 1 kΩ/2 kΩ divider 

           │  A4  → OLED SDA      │

           │  A5  → OLED SCL      │

           │  5V  → Encoder VCC   │

           │  5V  → OLED VCC      │

           │  GND → Common GND    │

           │VIN ← 12V │ ← A small 12 → 5 V buck module feeding the 5V pin might keep it cooler. 

           └─────────────────────┘

           ┌─────────────────────┐

           │   Rotary Encoder     │

           │  CLK → D2            │

           │  DT  → D3            │

           │  SW  → D6            │

           │  \+   → 5V            │

           │  GND → GND           │

           └─────────────────────┘

---

### 6.5 TMC2208 Current Setting

The **LDO-42STH48-1684MAC** is rated at **1.68A per phase**. The tuning load on the KP1-8 is negligible (\< 0.01 Nm). Run the driver at a fraction of the motor's rated current to minimise heat:

\$\$V\_{REF} \= I\_{RMS} \\times 1.41\$\$

**Recommended starting point: 0.6A RMS**

\$\$V\_{REF} \= 0.6 \\times 1.41 \\approx \\mathbf{0.85\\ V}\$\$

Measure VREF between the TMC2208 potentiometer wiper and GND with a multimeter while adjusting. Increase to **1.0V (≈ 0.71A RMS)** only if step skipping is observed.

---

### 6.6 Homing and Position Calibration

Because the system has no absolute position encoder, it must be homed at startup:

**Homing procedure (with microswitches):**

1. On power-up, run motor slowly **toward minimum capacitance** (CW direction when viewed from shaft end, typically increasing position number in firmware)  
   \[Firmware\] Direction: MIN \= minimum capacitance (cap fully open, 10m end) \= firmware position 0; homing runs in the NEGATIVE direction (decreasing position), not increasing. Whether that is CW or CCW depends on coil wiring — verify with the coupling loose. Firmware aborts homing if MAX trips (reversed DIR); fix by swapping one coil pair or INVERT\_DIR \= true.   
2. When the **MIN microswitch** triggers (Arduino D9 goes HIGH), stop immediately  
3. Call `stepper.setCurrentPosition(0)` — this is your zero reference  
4. Move **200–400 steps away** from the switch (clear the stop)  
5. `MAX_POSITION` \= empirically measured during first commissioning (see Section 10\)

**Without microswitches (software-only):**

- Set `MAX_POSITION` conservatively to **\~1,900,000** (direct drive) or **\~9,500,000** (5:1 gearbox) — about 5–10% inside the physical stops  
- Perform a manual zero by running to minimum and pressing a reset button in firmware

---

## 7\. Assembly: Coupling Loop & Feed

The coupling loop is how RF energy gets from your radio into the main loop.

### 7.1 Building the Faraday Coupling Loop

1. **Calculate the diameter** — The coupling loop should be **1/5 the diameter** of the main loop. For a 1.2m main loop → **\~24 cm diameter** coupling loop.  
     
2. **Cut the coax** — Take \~75 cm of RG-58 (or RG-213 for lower loss). This will form a circle of about 24 cm diameter.  
     
3. **Form the shield gap (Faraday loop):**  
     
   - Strip the outer jacket for about 1 cm at the **midpoint** (top) of the loop  
   - Carefully cut and separate the shield braid at this point — create a full gap in the shield  
   - **Do NOT cut the center conductor** — it must remain continuous  
   - This gap prevents electrostatic pickup while allowing magnetic coupling

   

4. **Connect the ends:**  
     
   - At one end, the coax center conductor and shield connect to the feedline's center and shield respectively (via SO-239 connector)  
   - At the other end, solder the center conductor to the shield of the first end (completing the loop)

        ── shield gap (1cm) ──

       /                       \\

      │                         │

      │    COUPLING LOOP        │

      │    (RG-58 coax)         │

       \\                       /

        ─┬─────────────────┬──

         │                 │

    \[shield─────────center\]

         │

    \[SO-239 connector\]

         │

    \[RG-58 feedline to radio\]

### 7.2 Positioning the Coupling Loop

5. **Mount at the bottom** of the main loop, directly opposite the capacitor gap  
6. **Center it inside** the main loop — the coupling loop plane should be parallel to (coplanar with) the main loop plane  
7. The coupling loop should **not** electrically touch the main loop  
8. **Rotate for best match** — Slightly tilting or rotating the coupling loop will adjust the impedance match. Aim for **SWR \< 1.5:1** at resonance. Once set, it stays set for the entire frequency range.

---

## 8\. Electronics & Firmware

### 8.1 Arduino Libraries Needed

Install via the Arduino IDE Library Manager:

- **AccelStepper** — for smooth motor acceleration/deceleration  
- **Adafruit\_SSD1306** \+ **Adafruit\_GFX** — for OLED display  
- **Encoder** (by Paul Stoffregen) — for rotary encoder

### 8.2 Firmware Code: [https\://github.com/robertrancz/magloop-antenna](https://github.com/robertrancz/magloop-antenna)

### 8.3 Bluetooth Control

**HC-05/HC-06 module:** connect TX/RX to Arduino pins (using SoftwareSerial) and duplicate the serial command processing. This lets you tune from your phone using any Bluetooth serial terminal app (e.g., "Serial Bluetooth Terminal" on Android).  
HC-05 on D11 (RX ← HC-05 TX) / D12 (TX → HC-05 RX via 1k/2k divider), 9600 baud, same commands as USB. Works with CR, LF, CRLF or no line ending. Replies are shortened while the motor runs.

### 8.4 Frequency Calibration

Once the antenna is assembled:

1. Connect an antenna analyzer (NanoVNA or similar) to the feedline  
2. Use coarse tuning to find resonance on each band  
3. Record the **stepper position** for each frequency  
4. Store these as presets in EEPROM — then you can jump directly to a band  
   \[Firmware\] Implemented: b40…b10 recall, s40…s10 save current position, or long-press (1.5 s) the encoder to save the active band. Stored in EEPROM. Commands: u/d, uu/dd, uuu/ddd, p, stop, home, h, or a number to go to an absolute position 

---

## 9\. Cable Management & Enclosure

### 9.1 Capacitor Enclosure (Top Box)

The enclosure at the top of the loop houses the capacitor and motor:

1. **Use a weatherproof ABS junction box, 360×150×110mm (minimum)** — this is the minimum to fit the LDO motor body (48mm) \+ coupling \+ KP1-8  
2. Drill holes on two opposite sides for the copper pipe ends to enter  
3. Drill a hole for the cable gland (5-wire control cable \+ 12V power cable)  
4. Mount the KP1-8 by its two brass bands in ring clamps on insulating posts (ASA/PTFE, nylon screws) — the bands are the terminals and are at RF high voltage.   
5. Mount the LDO NEMA17 to a **commercial aluminium NEMA17 bracket** or DIY angle bracket (see Section 6.2.2), bolted to the enclosure floor  
6. Connect motor shaft to KP1-8 shaft via **5mm→8mm beam coupling** (direct drive) or 8mm→10mm coupling (if gearbox used)  
7. Mount the travelling-nut rail with the two D2F-L switches on the enclosure floor under the LINK (see Section 6.2.5). 

### 9.2 Controller Box (Bottom or Indoor)

For indoor/balcony operation, you can split the electronics:

- **Option A (all-in-one):** Everything in the top box — simpler, shorter wires, but harder to access controls  
- **Option B (split — recommended):** Capacitor \+ motor in the top box; Arduino \+ TMC2208 \+ display \+ encoder in a separate small box at the base of the mast or inside the shack. Connect via a 7 or 8-wire cable \+ 12V power cable.

### 9.3 Cable Routing

\[RADIO\] ──RG-58──► \[SO-239 on coupling loop\] ──► coupling loop (bottom of main loop)

\[12V PSU\] ──DC cable──► \[Arduino box at base\]

                              │

                    4-wire stepper cable (3-5m)

                              │

                         \[Top box: TMC2208 \+ Motor \+ Capacitor\]

### 9.4 Cable Weatherproofing (for balcony use)

- Use **cable glands** (PG7/PG9) on all enclosure entry points  
- Wrap outdoor connectors with **self-amalgamating tape** (silicone rubber tape that fuses to itself)  
- Apply a bead of **silicone sealant** around cable gland entries  
- Use a **drip loop** on any cable entering a box — the cable should dip below the box entry point so water runs off

### 9.5 RF Considerations for the Control Box

- Add a **ferrite choke** (snap-on ferrite or several turns through a toroid) on:  
  - The stepper motor cable where it exits the top box  
  - The feedline coax near the coupling loop  
  - The 12V power cable  
- This prevents RF current from flowing on cable shields back to your equipment

---

## 10\. Tuning & Operation

Before the first power-up with the capacitor coupled: run homing with the coupling loose and verify direction, trip each switch by hand (FAULT expected), unplug MIN (homing must FAULT). Then calibrate MAX\_POSITION (see Section 6.6 comment) and save band presets. 

### 10.1 Initial Setup

1. Assemble the entire antenna with capacitor and coupling loop  
2. Connect your antenna analyzer (NanoVNA) to the feedline  
3. Set the NanoVNA to sweep 7–30 MHz  
4. Slowly turn the capacitor (via encoder/motor) and watch for a sharp SWR dip

### 10.2 Adjusting the Coupling Loop

- If the SWR minimum is above 2:1 → the coupling loop is too small or too far away  
- If the SWR minimum never goes below 1.5:1 → try rotating/tilting the coupling loop slightly  
- The coupling loop adjustment is a **one-time** setup; once optimized, it stays fixed

### 10.3 Normal Operation

1. Turn on the controller  
2. Select a band preset (or use coarse tuning to get close)  
3. Transmit a carrier at low power (5–10W) while watching SWR  
4. Fine-tune with the encoder until SWR is minimized  
5. Increase to operating power, **respecting band power limits for the Polaron KP1-8 (5 kV):**

| Band | Max Power (KP1-8, 5 kV) | Notes |
| :---- | :---- | :---- |
| 40m | **30W** |  |
| 30m | **70W** | Safe within 5 kV |
| 20m | **35W** | Peak \~6,800V at 100W — derate to 55–60W |
| 17m | **70W** |  |
| 15m–10m | **100W** | Safe |

### 10.4 Bandwidth Expectations

| Band | Approximate \-3dB Bandwidth | Efficiency (1.2m loop) | Notes |
| :---- | :---- | :---- | :---- |
| 40m (7 MHz) | \~4–5 kHz | \~22% | **Bonus band.** Very narrow — FT8/CW only. Retune for any QSY. |
| 30m (10 MHz) | \~10–14 kHz | \~45% | Good for digital modes |
| 20m (14 MHz) | \~22–28 kHz | \~78% | **Primary band** — excellent performance |
| 17m (18 MHz) | \~35–45 kHz | \~88% | Excellent |
| 15m (21 MHz) | \~45–60 kHz | \~92% | Near full-size performance |
| 12m (24 MHz) | \~55–75 kHz | \~95% | Excellent — slight pattern broadening |
| 10m (28 MHz) | \~70–100 kHz | \~96% | Superb efficiency. May not reach upper 10m (\>28.5 MHz) — see Section 1 edge case note. |

> On 40m, the \~4–5 kHz bandwidth means even a small QSY requires retuning — the motorized system is essential here. On 20m+, you may be able to tune once and operate across a reasonable portion of the band.

---

## 11\. Safety Notes

### RF Safety — HIGH VOLTAGE

⚠️ **Voltages across the Polaron KP1-8 can reach \~6,800 V peak at 100W on 20m, and \~6,100 V peak at 60W on 40m.** The capacitor’s 5 kV working voltage means these limits must be respected — see the Power Strategy table in Section 1\. Regardless of power level, never touch the capacitor or top of the loop while transmitting.

- **NEVER touch the capacitor or the top of the loop while transmitting — on ANY band, at ANY power**  
- Keep the capacitor gap (top of loop) at least **2 meters away from people** during transmission and/or an acrylic shield over the top box.   
- Use a non-conductive mast (PVC, fibreglass) — never metallic  
- If operating indoors, ensure no one can accidentally touch the top section  
- Consider a **safety interlock** that disables TX if anyone is near the antenna  
- RF exposure limits apply — consult your country's regulations (ANCOM in Romania)  
- **40m is particularly dangerous** because the high Q generates extreme voltages even at low power — 60W on 40m produces similar or higher voltages than 100W on 20m

### RF Exposure

The near field of a magnetic loop is **very strong**:

- Maintain at least **1–2 meters distance** from the antenna while transmitting on any band  
- On a balcony, the antenna should be on the railing or above head height  
- Indoor operation at 60–100W is **not recommended** unless the antenna is in a separate room with nobody in it  
- For indoor use with people nearby, reduce to **10–25W** and use digital modes (FT8, JS8Call) or CW which are effective at low power  
- On 40m at 60W, the **circulating current is \~35A** — this produces significant magnetic field intensity near the antenna

### Electrical Safety

- Use a proper 12V power supply — do not hack together a high-current supply  
- Add a fuse (2A) on the 12V line  
- Ensure the TMC2208 heatsink is adequate (stick-on aluminium heatsink included with most modules)  
- The LDO-42STH48-1684MAC rated at 1.68A — run it at 0.6A RMS (VREF \~0.85V) for this application; de-energise coils when idle via the EN pin  
- The motor does not need to run continuously — set EN pin HIGH when not tuning

### Grounding

- If mounted outdoors on a balcony, ground the base of the loop to your station ground  
- The coupling loop shield provides a path to ground via the coax — ensure your radio is properly grounded  
- For lightning protection: disconnect the antenna when not in use

---

## References & Further Reading

| Resource | URL |
| :---- | :---- |
| AA5TB — Small Transmitting Loop Antennas | [https\://aa5tb.com/loop.html](https://aa5tb.com/loop.html) |
| 66pacific Magnetic Loop Calculator | [http\://www\.66pacific.com/calculators/small-transmitting-loop-antenna-calculator.aspx](http://www.66pacific.com/calculators/small-transmitting-loop-antenna-calculator.aspx) |
| Eric Sorensen (AA7AJ) — Loop Antenna Build | [http\://esorensen.com/loop\_antenna\_intro/](http://esorensen.com/loop_antenna_intro/) |
| sh123 MagLoop Controller (GitHub) | [https\://github.com/sh123/magloop\_controller](https://github.com/sh123/magloop_controller) |
| Hackaday — Building a Magnetic Loop Antenna | [https\://hackaday.com/2019/03/17/building-a-magnetic-loop-antenna/](https://hackaday.com/2019/03/17/building-a-magnetic-loop-antenna/) |
| Hackaday — Remotely Tuned Magnetic Loop | [https\://hackaday.com/2017/05/08/a-remotely-tuned-magnetic-loop-antenna/](https://hackaday.com/2017/05/08/a-remotely-tuned-magnetic-loop-antenna/) |
| LDO Motors — 42STH48-1684MAC datasheet | [https\://ldomotors.com](https://ldomotors.com) |
| AccelStepper library | [https\://www\.airspayce.com/mikem/arduino/AccelStepper/](https://www.airspayce.com/mikem/arduino/AccelStepper/) |
| TMCStepper library (UART config) | [https\://github.com/teemuatlut/TMCStepper](https://github.com/teemuatlut/TMCStepper) |
| M0UKD — Mag Loop Build | [https\://m0ukd.com/homebrew/antennas/magnetic-loop-antennas/](https://m0ukd.com/homebrew/antennas/magnetic-loop-antennas/) |
| WiMo — Magnetic Loop Info | [https\://www\.wimo.com/en/antennas/hf-antennas/magnetic-loop-antennas](https://www.wimo.com/en/antennas/hf-antennas/magnetic-loop-antennas) |
| ARRL Antenna Handbook | Check your local library or ARRL website |

---

*Document updated April 2026 for Polaron KP1-8 (4–250 pF, 5 kV, 10 mm shaft) and LDO-42STH48-1684MAC (0.9°/step, 1.68A). No 3D-printed parts. Verify all links and product availability before purchasing. 73\!*

## Buying List

### 1\. Motorized Tuning System 

Item				Specification			Qty	Already bought  
Arduino Nano clone		ATmega328P \+ CH340	1	✔️  
TMC2208 stepper driver	With heatsink			1	✔️  
KY-040 rotary encoder	With push button		1	✔️  
OLED display	0.96" I2C 	SSD1306 128×64		1	✔️  
HC-05 Bluetooth module 	Serial TTL			1	✔️  
DC barrel jack	5.5×2.1mm	panel mount			1	✔️  
Perfboard			7×9cm				1	✔️  
Electrolytic capacitor		100µF / 50V			3	✔️  
Ceramic capacitors		100nF (0.1µF)			10	✔️  
Resistors			1kΩ, 2kΩ, 10kΩ — ×5 each	15	✔️  
Pin headers			2.54mm male \+ female strips	2 sets	✔️  
PCB screw terminals		2-pin, 5.08mm pitch		6	✔️  
Snap-action switches		Omron D2F-L or equivalent	2	✔️  
Hookup wire 24 AWG		Assorted colours \~5m		1	✔️  
Solder				0.8mm, rosin core		1 reel	✔️  
Snap-on ferrite cores		5mm cable diameter		3	✔️

*Order from AliExpress*  
Item					Specification				Qty	Purchased  
LDO-42STH48-1684MAC motor	NEMA17, 0.9°/step, 1.68A, 48mm	1	✔️  
Beam coupling			5mm × 8mm bore, D19L25		1	✔️  
NEMA17 aluminium L-bracket	Pre-drilled, commercial		1	✔️  
IP65 ABS enclosure (top box)	220×150×80mm minimum		1	✔️  
ABS project box (control box)		120×80×40mm			1	✔️  
8-wire shielded cable			0.5mm² conductors, 5m		1	  
Cable glands				PG7 (×3) \+ PG9 (×2) assortment	5	✔️  
Heat shrink tubing			Assorted sizes, pack			1	✔️  
12V 2A DC power adapter		5.5×2.1mm plug			1	✔️

*From Dedeman / Leroy Merlin (hardware)*  
Item				Specification				Qty		Purchased  
Aluminium flat plate		3–5mm thick, \~100×80mm		1		✔️  
Aluminium L-profile, 300mm						1		✔️  
M3 stainless bolts \+ nuts	M3×8mm (×10), M3×12mm (×5)	1 set		**TODO**  
M4 stainless bolts \+ nuts	M4×10mm (×10), M4×16mm (×5)	1 set		**TODO**  
Zip ties				Assorted				1 pack		✔️

### 2\. Antenna

**Main Loop — From Dedeman / Leroy Merlin / Plumbing Shop**  
Item					Specification			Qty		Est. Price  
Copper pipe · Țeavă cupru		22mm OD, 3.8m length	1		**TODO**  
Copper pipe end caps 		Dopuri cupru 22mm		2		**TODO**  
Stainless hose clamps		Coliere inox 22mm–32mm	4		✔️  
PVC pipe · Țeavă PVC		32mm or 40mm, 1.5m 	1		**TODO**  
PVC T-junction · Teu PVC		Matching pipe diameter	1		**TODO**  
Self-amalgamating tape		Bandă autovulcanizantă	1		✔️  
Silicone sealant			Clear, outdoor grade		1 tube		**TODO**

**Main Loop & Feedline**  
Item					Specification			Qty		Est. Price  
SO-239 chassis mount connector	UHF female, panel mount	2		✔️  
Copper braid / strap			15mm wide, \~15cm length	2 pcs		✔️  
RG-58 coaxial cable	50Ω, 5m 	(feedline radio→antenna)	1		✔️  
PL-259 UHF connectors		Solder type for RG-58	4			✔️  
RG-58 coax (coupling loop)		50Ω, \~1m offcut		1		✔️

### 3\. Tools You'll Need (if not already owned)

Tool					Notes  
NanoVNA antenna analyzer		Essential for tuning					✔️  
Step drill bit				For drilling enclosure holes cleanly			✔️  
M3 and M4 tap set			For threading aluminium bracket holes		✔️  
Multimeter				Already own one, presumably			✔️  
Needle file (pilă)			For filing a flat on the KP1-8 shaft			✔️

