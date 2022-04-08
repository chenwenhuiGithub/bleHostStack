#ifndef SM_H
#define SM_H

#include <stdint.h>

void sm_c1(uint8_t* k, uint8_t* r, uint8_t* preq, uint8_t *pres, uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out_confirm);
void sm_s1(uint8_t* k, uint8_t* r1, uint8_t* r2, uint8_t* out_stk);
void sm_f4(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t z, uint8_t* out_confirm);
void sm_f5(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_mackey, uint8_t* out_ltk);
void sm_f6(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t* r, uint8_t* iocap, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_dhkey_check);
void sm_g2(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t* y, uint32_t* out_passkey);

#endif // SM_H
