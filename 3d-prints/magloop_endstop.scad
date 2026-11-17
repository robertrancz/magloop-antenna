// ============================================================================
//  MagLoop KP1-8 travelling-nut end-stop mechanism  (inline layout "A")
//  - LINK : one printed part between the 5x8 beam coupling and the KP1-8 shaft
//           [8 mm stub] [thread] [plain insulator] [8 mm shaft clamp]
//  - NUT  : threaded nut with a fin; the fin rides in the rail slot so the
//           nut cannot rotate and travels one PITCH per shaft turn
//  - RAIL : bolts to the enclosure floor under the link; guide slot + two
//           D2F-L switch seats with slotted holes for fine adjustment
//  - BASE : RAIL + printed NEMA17 face-plate + body pad on ONE floor plate,
//           so motor shaft and nut guide share the same axis_h (replaces the
//           commercial L-bracket and a separate pad). Use BASE or RAIL, not both.
//
//  Print in PETG, 100 % infill (insulator: no internal air voids), 0.12-0.16 mm
//  layers. LINK and NUT print with the thread axis vertical (as exported).
//  Render one part:  set PART below, F6, export STL.
//  CLI:  openscad -D 'PART="link"' -o link.stl magloop_endstop.scad
// ============================================================================

PART = "assembly";          // "assembly" | "link" | "nut" | "rail" | "base" | "print_all"

/* [Capacitor & drive] */
cap_turns      = 22.5;      // KP1-8 full mechanical travel (measured)
trip_margin    = 0.75;      // turns before each mechanical stop where switches trip
cap_shaft_d    = 8.0;       // KP1-8 shaft (measured 8 mm; file a flat for the set screw)
axis_h         = 50;        // shaft axis height above the floor: KP1-8 radius 35 + 15 mm (ring clamp + clearance)

/* [Thread] */
pitch          = 1.5;       // mm per turn = nut travel per shaft turn (firmware assumes ~1.5)
thread_d       = 12;        // major diameter of the printed thread
thread_h       = 0.6 * pitch;   // thread depth (radial)
thread_clr     = 0.35;      // radial clearance nut <-> screw (tune for your printer)

/* [Link] */
clamp_len      = 20;        // KP1-8 shaft clamp length
clamp_d        = 22;
cap_bore_clr   = 0.2;       // printed holes come out small
ins_len        = 30;        // plain insulating section (keeps rail + switches away from the HV clamp)
ins_d          = 16;
stub_d         = 8.0;       // goes into the 8 mm bore of the 5x8 beam coupling
stub_len       = 14;
stub_flat      = 0.5;       // D-flat depth for the coupling set screw
steel_stub     = false;     // true = 8 mm blind hole for a glued-in steel rod instead of a printed stub

/* [Nut & rail] */
nut_len        = 12;
nut_wall       = 4;
fin_t          = 6;         // fin thickness (rides in the rail slot)
slot_clr       = 0.4;       // per side
base_t         = 4;
rail_w         = 36;
guide_wall_t   = 4;
sw_zone        = 20;        // switch seat length at each rail end

/* [Motor & coupling - BASE part] */
mot_w          = 42.3;      // NEMA17 body (LDO drawing: 42.3 max - measure yours, use the real value)
mot_len        = 48;        // body length (LDO-42STH48: 48 max)
mot_hole_pitch = 31;        // M3 pattern on the front face
mot_boss_d     = 22;        // front boss (2 mm high)
mot_shaft_len  = 24;        // from mounting face to shaft tip
cpl_len        = 25;        // D19L25 beam coupling
cpl_bore_depth = 12.5;      // depth of each bore (half the coupling)
shaft_gap      = 1.5;       // gap between motor shaft tip and LINK stub end inside the coupling
face_t         = 6;         // printed motor face-plate thickness
pad_clr        = 0.5;       // gap under the motor body -> shim to touch (body pad is support only)
pad_len        = 30;        // body pad length (stops short of the rear connector)
base_w         = 60;        // base width under the motor

/* [Omron D2F-L - verify against your switches] */
sw_hole_d      = 2.2;       // M2
sw_hole_pitch  = 6.5;
sw_len         = 12.8;
sw_depth       = 6.5;       // body depth from mounting face to lever face
sw_adj         = 8;         // slot length for fine adjustment along the shaft

$fn = 64;
eps = 0.01;

// ── Derived ────────────────────────────────────────────────────────────────
full_travel  = cap_turns * pitch;                          // nut travel over the full cap range
trip_travel  = (cap_turns - 2 * trip_margin) * pitch;      // distance between switch trip points
thread_len   = ceil(full_travel + nut_len + 4);            // + 2 mm spare each end
link_len     = clamp_len + ins_len + thread_len + (steel_stub ? 0 : stub_len);
nut_d        = thread_d + 2 * nut_wall;
fin_reach    = axis_h - base_t - 1;                        // axis -> fin bottom
wall_top     = min(axis_h - nut_d / 2 - 1.5, base_t + 14);
lever_gap    = trip_travel + nut_len;                      // distance between the two lever faces
rail_len     = lever_gap + 2 * sw_zone;
// Shaft-line positions (X from the LINK clamp face on the KP1-8 side)
x_thread0    = clamp_len + ins_len;
rail_x0      = x_thread0 + (thread_len - nut_len) / 2 - trip_travel / 2 - sw_zone;
cpl_x0       = link_len - cpl_bore_depth;                  // coupling: stub fully in the 8 mm bore
mot_face_x   = cpl_x0 + cpl_bore_depth + shaft_gap + mot_shaft_len;   // motor mounting face
pad_h        = axis_h - mot_w / 2;                         // floor -> motor body underside

echo(str("BASE: motor face at X = ", mot_face_x, " mm, body underside at ", pad_h,
         " mm above the floor (pad printed ", pad_h - pad_clr, " + shims)"));
echo(str("BASE: motor shaft inside coupling = ", mot_shaft_len - (mot_face_x - (cpl_x0 + cpl_len)), " mm"));

echo(str("Nut travel full range = ", full_travel, " mm, between trip points = ", trip_travel, " mm"));
echo(str("Link length = ", link_len, " mm (thread ", thread_len, " mm), rail length = ", rail_len, " mm"));
echo(str("Wall top above floor = ", wall_top, " mm (needs > ", base_t + 5, ")"));
assert(wall_top > base_t + 5, "axis_h too low for nut + guide walls: increase axis_h or reduce nut_wall");

// ── Thread (single start, right-hand, trapezoid-ish 45 deg flanks) ─────────
function prof(u) =                                         // u in [0,1) along one pitch -> 0..1 radial
    u < 0.25  ? 0 :
    u < 0.5   ? (u - 0.25) / 0.25 :
    u < 0.625 ? 1 :
    u < 0.875 ? 1 - (u - 0.625) / 0.25 : 0;

module thread_section(r_min, depth, n = 90) {
    polygon([for (i = [0 : n - 1])
        let(a = 360 * i / n, u = 1 - i / n - floor(1 - i / n))
        (r_min + depth * prof(u)) * [cos(a), sin(a)]]);
}

module thread(len, r_maj, depth, extra = 0) {
    linear_extrude(height = len, twist = -360 * len / pitch,
                   slices = ceil(len / pitch * 24), convexity = 6)
        thread_section(r_maj - depth + extra, depth);
}

// ── LINK ───────────────────────────────────────────────────────────────────
module link() {
    difference() {
        union() {
            // KP1-8 clamp (bottom when printing)
            cylinder(d = clamp_d, h = clamp_len);
            // insulator with chamfered shoulders
            translate([0, 0, clamp_len - eps]) cylinder(d = ins_d, h = ins_len + eps);
            // threaded section
            translate([0, 0, clamp_len + ins_len - eps])
                thread(thread_len + eps, thread_d / 2, thread_h);
            // motor-side stub
            if (!steel_stub)
                translate([0, 0, clamp_len + ins_len + thread_len - eps])
                    difference() {
                        cylinder(d = stub_d - 0.1, h = stub_len + eps);
                        translate([stub_d / 2 - stub_flat, -stub_d, 2]) cube([stub_d, 2 * stub_d, stub_len]);
                    }
        }
        // KP1-8 shaft bore (stops 2 mm short of the insulator)
        translate([0, 0, -eps]) cylinder(d = cap_shaft_d + cap_bore_clr, h = clamp_len - 2);
        // clamp slit
        translate([0, -0.75, -eps]) cube([clamp_d, 1.5, clamp_len - 4]);
        // M3 clamp bolt across the slit, head counterbore + nut trap
        translate([clamp_d / 2 - 4.5, 0, clamp_len / 2 - 2]) rotate([90, 0, 0]) {
            cylinder(d = 3.4, h = clamp_d, center = true);
            translate([0, 0, 4]) cylinder(d = 6.2, h = clamp_d);
            translate([0, 0, -4 - clamp_d]) cylinder(d = 6.4, h = clamp_d, $fn = 6);
        }
        // radial M3 set-screw hole onto the filed flat (2.5 mm -> tap M3)
        translate([0, 0, clamp_len / 2 - 2]) rotate([0, -90, 0]) cylinder(d = 2.5, h = clamp_d);
        // optional steel stub hole
        if (steel_stub)
            translate([0, 0, link_len - 15]) cylinder(d = 8.1, h = 15 + eps);
    }
}

// ── NUT (printed with thread axis vertical, fin along -Y) ──────────────────
module nut() {
    difference() {
        union() {
            cylinder(d = nut_d, h = nut_len);
            translate([-fin_t / 2, -fin_reach, 0]) cube([fin_t, fin_reach, nut_len]);
        }
        translate([0, 0, -1]) thread(nut_len + 2, thread_d / 2, thread_h, thread_clr);
        // lead-in chamfers
        translate([0, 0, -eps]) cylinder(d1 = thread_d + 1, d2 = thread_d - 2 * thread_h, h = 1.2);
        translate([0, 0, nut_len - 1.2 + eps]) cylinder(d1 = thread_d - 2 * thread_h, d2 = thread_d + 1, h = 1.2);
    }
}

// ── RAIL (X = shaft axis, origin at rail start, floor at z = 0) ─────────────
module sw_slots(x0) {
    // two M2 slots along X, spaced sw_hole_pitch across the rail, + M2 nut channel underneath
    for (y = [-sw_hole_pitch / 2, sw_hole_pitch / 2])
        hull() for (dx = [0, sw_adj]) translate([x0 + dx, y, -eps]) {
            cylinder(d = sw_hole_d, h = base_t + 2);
        }
    for (y = [-sw_hole_pitch / 2, sw_hole_pitch / 2])
        hull() for (dx = [0, sw_adj]) translate([x0 + dx, y, -eps]) cylinder(d = 4.6, h = 1.8, $fn = 6);
}

module rail() {
    slot_w = fin_t + 2 * slot_clr;
    difference() {
        union() {
            translate([0, -rail_w / 2, 0]) cube([rail_len, rail_w, base_t]);
            // guide walls between the switch seats
            for (s = [-1, 1])
                translate([sw_zone, s > 0 ? slot_w / 2 : -slot_w / 2 - guide_wall_t, 0])
                    cube([lever_gap, guide_wall_t, wall_top]);
        }
        // switch seats: switches lie flat, long side across the rail, lever facing the fin
        sw_slots(sw_zone - sw_depth / 2 - sw_adj / 2);                     // MIN-end seat
        sw_slots(rail_len - sw_zone + sw_depth / 2 - sw_adj / 2);          // MAX-end seat
        // M4 floor bolts
        for (x = [rail_len * 0.3, rail_len * 0.7], y = [-rail_w / 2 + 5, rail_w / 2 - 5])
            translate([x, y, -eps]) cylinder(d = 4.4, h = base_t + 2);
    }
}

// ── BASE: rail + motor face-plate + body pad on one floor plate ─────────────
// Horizontal holes as teardrops so they print without supports (plate printed upright)
module teardrop_x(d, len) {
    rotate([0, 90, 0]) linear_extrude(height = len, center = true)
        rotate(90) union() {
            circle(d = d);
            polygon([[0, d / 2 / sin(45)], [-d / 2 * cos(45), d / 2 * sin(45)], [d / 2 * cos(45), d / 2 * sin(45)]]);
        }
}

module motor_mount() {
    // local X: 0 = motor mounting face; plate sits at -face_t..0, motor body at 0..mot_len
    plate_h = axis_h + mot_hole_pitch / 2 + 6;
    gusset_y = mot_w / 2 + 1.5;
    difference() {
        union() {
            translate([-face_t, -base_w / 2, 0]) cube([face_t, base_w, plate_h]);
            // body pad (support only: pad_clr gap, shim to touch)
            translate([4, -15, 0]) cube([pad_len, 30, pad_h - pad_clr]);
            // side gussets outside the motor body
            for (s = [-1, 1])
                translate([0, s > 0 ? gusset_y : -gusset_y - 5, 0])
                    rotate([90, 0, 0]) translate([0, 0, -5])
                        linear_extrude(height = 5)
                            polygon([[-eps, 0], [mot_len * 0.8, 0], [-eps, plate_h - 4]]);
        }
        // shaft + boss clearance, 4 x M3 (screw from the coupling side: M3 x 10)
        translate([-face_t / 2, 0, axis_h]) teardrop_x(mot_boss_d + 1, face_t + 2);
        for (y = [-1, 1], z = [-1, 1])
            translate([-face_t / 2, y * mot_hole_pitch / 2, axis_h + z * mot_hole_pitch / 2])
                teardrop_x(3.4, face_t + 2);
        // M3 head counterbores on the coupling side, 3 mm deep
        for (y = [-1, 1], z = [-1, 1])
            translate([-face_t - eps, y * mot_hole_pitch / 2, axis_h + z * mot_hole_pitch / 2])
                rotate([0, 90, 0]) cylinder(d = 6.2, h = 3);
        // zip-tie slot through the pad (optional body strap)
        translate([4 + pad_len / 2 - 2, -20, pad_h - pad_clr - 6]) cube([4, 40, 2.5]);
    }
}

module base() {
    // origin = rail start (same as rail()), X along the shaft towards the motor
    m0 = mot_face_x - rail_x0;                                   // motor face in base coords
    union() {
        rail();
        difference() {
            union() {
                // floor bridge rail -> motor area, and motor area floor
                translate([rail_len - eps, -rail_w / 2, 0]) cube([m0 - face_t - rail_len + 2 * eps, rail_w, base_t]);
                translate([m0 - face_t, -base_w / 2, 0]) cube([face_t + mot_len * 0.8, base_w, base_t]);
            }
            for (x = [m0 + 10, m0 + mot_len * 0.8 - 8], y = [-base_w / 2 + 5, base_w / 2 - 5])
                translate([x, y, -eps]) cylinder(d = 4.4, h = base_t + 2);
        }
        translate([m0, 0, 0]) motor_mount();
    }
}

// ── Ghost switch (for assembly view only) ──────────────────────────────────
module ghost_switch(x_lever_face, dir) {
    color("dimgray", 0.8)
        translate([dir > 0 ? x_lever_face - sw_depth : x_lever_face, -sw_len / 2, base_t])
            cube([sw_depth, sw_len, 5.8]);
    color("silver")
        translate([dir > 0 ? x_lever_face : x_lever_face - 1, -sw_len / 2 + 1, base_t + 1.5])
            cube([1, sw_len - 2, 2.8]);
}

// ── Output ─────────────────────────────────────────────────────────────────
if (PART == "link") link();
else if (PART == "nut") nut();
else if (PART == "rail") rail();
else if (PART == "base") base();
else if (PART == "print_all") {
    link();
    translate([clamp_d + 10, 0, 0]) nut();
    translate([-rail_len / 2, -clamp_d - base_w, 0]) base();
}
else {
    // Assembly: link along +X (cap at X = 0), nut at mid travel, BASE on the floor
    color("orange", 0.9) translate([0, 0, axis_h]) rotate([0, 90, 0]) link();
    color("deepskyblue") translate([x_thread0 + (thread_len - nut_len) / 2, 0, axis_h])
        rotate([0, 90, 0]) rotate([0, 0, 90]) nut();
    translate([rail_x0, 0, 0]) {
        color("lightgreen") base();
        ghost_switch(sw_zone, 1);
        ghost_switch(rail_len - sw_zone, -1);
    }
    color("gray", 0.5) translate([-170, -45, -2]) cube([link_len + 240, 90, 2]);   // enclosure floor
    // KP1-8 (sketch, measured 70 x 160 incl. shaft): drive head, upper band, glass, lower band
    translate([-160 + clamp_len - 2, 0, axis_h]) rotate([0, 90, 0]) {
        color("goldenrod") cylinder(d = 70, h = 26);                                   // lower brass band
        color("lightcyan", 0.5) translate([0, 0, 26]) cylinder(d = 66, h = 60);        // glass
        color("goldenrod") translate([0, 0, 86]) cylinder(d = 70, h = 34);             // upper brass band
        color("sienna") translate([0, 0, 120]) cylinder(d = 40, h = 22);               // drive head
        color("silver") translate([0, 0, 142]) cylinder(d = 8, h = 18);                // shaft
    }
    color("silver") translate([cpl_x0, 0, axis_h]) rotate([0, 90, 0]) cylinder(d = 19, h = cpl_len);  // beam coupling
    color("black") translate([mot_face_x, -mot_w / 2, axis_h - mot_w / 2]) cube([mot_len, mot_w, mot_w]);  // NEMA17
    color("silver") translate([mot_face_x - mot_shaft_len, 0, axis_h]) rotate([0, 90, 0]) cylinder(d = 5, h = mot_shaft_len);
}
