mul v1.4s, v0.4s, v1.4s
and v1.16b, v1.16b, v3.16b
xtn v1.4h, v1.4s
umlal v0.4s, v1.4h, v2.4h
ushr v0.4s, v0.4s, #12
xtn v0.4h, v0.4s