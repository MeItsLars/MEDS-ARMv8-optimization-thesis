// a is saved in v0.8h
// MEDS_p is saved in v1.8h

cmhs v2.8h, v0.8h, v1.8h
and v2.16b, v2.16b, v1.16b
sub v0.8h, v0.8h, v2.8h