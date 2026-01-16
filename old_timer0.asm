_timer0:
	ar7 = 0x0f
	ar6 = 0x0e
	ar5 = 0x0d
	ar4 = 0x0c
	ar3 = 0x0b
	ar2 = 0x0a
	ar1 = 0x09
	ar0 = 0x08
	push	acc
	push	b
	push	dpl
	push	dph
	push	psw
	mov	psw,#0x08
;	src/main.c:232: TH0 = 0xEF;  // 2.5ms
	mov	_TH0,#0xef
;	src/main.c:233: TL0 = 0xB9;
	mov	_TL0,#0xb9
;	src/main.c:235: TR0 = 1;  // restart timer 0
;	assignBit
	setb	_TR0
;	src/main.c:237: P3_5 = 1;
;	assignBit
	setb	_P3_5
;	src/main.c:239: uint8_t* m_ptr = matrix;
	mov	r6,_matrix
	mov	r7,(_matrix + 1)
;	src/main.c:240: uint8_t* f_ptr = framebuffer[line_counter];
	mov	a,_timer0_line_counter_10000_31
	mov	b,#0x10
	mul	ab
	add	a, #_framebuffer
	mov	r4,a
	mov	a,#(_framebuffer >> 8)
	addc	a, b
	mov	r5,a
;	src/main.c:242: for (uint8_t i = 0; i < 4; i++) {
	mov	_timer0_i_20001_33,#0x00
00106$:
	mov	a,#0x100 - 0x04
	add	a,_timer0_i_20001_33
	jnc	00155$
	ljmp	00101$
00155$:
;	src/main.c:243: *(m_ptr + 8) = 0;
	mov	a,#0x08
	add	a, r6
	mov	dpl,a
	clr	a
	addc	a, r7
	mov	dph,a
	clr	a
	movx	@dptr,a
;	src/main.c:245: *m_ptr++ = *f_ptr++;
	mov	dpl,r4
	mov	dph,r5
	movx	a,@dptr
	mov	r2,a
	mov	a,#0x01
	add	a, r4
	mov	_timer0_sloc0_1_0,a
	clr	a
	addc	a, r5
	mov	(_timer0_sloc0_1_0 + 1),a
	mov	dpl,r6
	mov	dph,r7
	mov	a,r2
	movx	@dptr,a
	mov	a,#0x01
	add	a, r6
	mov	r2,a
	clr	a
	addc	a, r7
	mov	r3,a
;	src/main.c:246: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00156$
	inc	(_timer0_sloc0_1_0 + 1)
00156$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00157$
	inc	r3
00157$:
;	src/main.c:247: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00158$
	inc	(_timer0_sloc0_1_0 + 1)
00158$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00159$
	inc	r3
00159$:
;	src/main.c:248: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00160$
	inc	(_timer0_sloc0_1_0 + 1)
00160$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00161$
	inc	r3
00161$:
;	src/main.c:249: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	mov	a,#0x01
	add	a, _timer0_sloc0_1_0
	mov	r0,a
	clr	a
	addc	a, (_timer0_sloc0_1_0 + 1)
	mov	r1,a
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	mov	a,#0x01
	add	a, r2
	mov	_timer0_sloc0_1_0,a
	clr	a
	addc	a, r3
	mov	(_timer0_sloc0_1_0 + 1),a
;	src/main.c:250: *m_ptr++ = *f_ptr++;
	mov	dpl,r0
	mov	dph,r1
	movx	a,@dptr
	mov	r3,a
	inc	r0
	cjne	r0,#0x00,00162$
	inc	r1
00162$:
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	mov	a,r3
	movx	@dptr,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00163$
	inc	(_timer0_sloc0_1_0 + 1)
00163$:
;	src/main.c:251: *m_ptr++ = *f_ptr++;
	mov	dpl,r0
	mov	dph,r1
	movx	a,@dptr
	mov	r3,a
	inc	r0
	cjne	r0,#0x00,00164$
	inc	r1
00164$:
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	mov	a,r3
	movx	@dptr,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00165$
	inc	(_timer0_sloc0_1_0 + 1)
00165$:
;	src/main.c:252: *m_ptr++ = *f_ptr++;
	mov	dpl,r0
	mov	dph,r1
	movx	a,@dptr
	mov	r3,a
	mov	a,#0x01
	add	a, r0
	mov	r4,a
	clr	a
	addc	a, r1
	mov	r5,a
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	mov	a,r3
	movx	@dptr,a
	mov	a,#0x01
	add	a, _timer0_sloc0_1_0
	mov	r2,a
	clr	a
	addc	a, (_timer0_sloc0_1_0 + 1)
	mov	r3,a
;	src/main.c:254: *m_ptr++ = line_counter | 0x08;
	mov	a,#0x08
	orl	a,_timer0_line_counter_10000_31
	mov	dpl,r2
	mov	dph,r3
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00166$
	inc	r3
00166$:
;	src/main.c:256: m_ptr += 7;
	mov	a,#0x07
	add	a, r2
	mov	r6,a
	clr	a
	addc	a, r3
	mov	r7,a
;	src/main.c:242: for (uint8_t i = 0; i < 4; i++) {
	inc	_timer0_i_20001_33
	ljmp	00106$
00101$:
;	src/main.c:259: f_ptr += 96;
	mov	a,#0x60
	add	a, r4
	mov	r4,a
	clr	a
	addc	a, r5
	mov	r5,a
;	src/main.c:261: for (uint8_t i = 0; i < 4; i++) {
	mov	_timer0_i_20001_35,#0x00
00109$:
	mov	a,#0x100 - 0x04
	add	a,_timer0_i_20001_35
	jnc	00167$
	ljmp	00102$
00167$:
;	src/main.c:262: *(m_ptr + 8) = 0;
	mov	a,#0x08
	add	a, r6
	mov	dpl,a
	clr	a
	addc	a, r7
	mov	dph,a
	clr	a
	movx	@dptr,a
;	src/main.c:264: *m_ptr++ = *f_ptr++;
	mov	dpl,r4
	mov	dph,r5
	movx	a,@dptr
	mov	r2,a
	mov	a,#0x01
	add	a, r4
	mov	_timer0_sloc0_1_0,a
	clr	a
	addc	a, r5
	mov	(_timer0_sloc0_1_0 + 1),a
	mov	dpl,r6
	mov	dph,r7
	mov	a,r2
	movx	@dptr,a
	mov	a,#0x01
	add	a, r6
	mov	r2,a
	clr	a
	addc	a, r7
	mov	r3,a
;	src/main.c:265: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	mov	a,#0x01
	add	a, _timer0_sloc0_1_0
	mov	_timer0_f_ptr_10001_32,a
	clr	a
	addc	a, (_timer0_sloc0_1_0 + 1)
	mov	(_timer0_f_ptr_10001_32 + 1),a
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00168$
	inc	r3
00168$:
;	src/main.c:266: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_f_ptr_10001_32
	mov	dph,(_timer0_f_ptr_10001_32 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	mov	a,#0x01
	add	a, _timer0_f_ptr_10001_32
	mov	r0,a
	clr	a
	addc	a, (_timer0_f_ptr_10001_32 + 1)
	mov	r1,a
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	mov	a,#0x01
	add	a, r2
	mov	_timer0_m_ptr_10001_32,a
	clr	a
	addc	a, r3
	mov	(_timer0_m_ptr_10001_32 + 1),a
;	src/main.c:267: *m_ptr++ = *f_ptr++;
	mov	dpl,r0
	mov	dph,r1
	movx	a,@dptr
	mov	r3,a
	mov	a,#0x01
	add	a, r0
	mov	_timer0_sloc0_1_0,a
	clr	a
	addc	a, r1
	mov	(_timer0_sloc0_1_0 + 1),a
	mov	dpl,_timer0_m_ptr_10001_32
	mov	dph,(_timer0_m_ptr_10001_32 + 1)
	mov	a,r3
	movx	@dptr,a
	mov	a,#0x01
	add	a, _timer0_m_ptr_10001_32
	mov	r2,a
	clr	a
	addc	a, (_timer0_m_ptr_10001_32 + 1)
	mov	r3,a
;	src/main.c:268: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00169$
	inc	(_timer0_sloc0_1_0 + 1)
00169$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00170$
	inc	r3
00170$:
;	src/main.c:269: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00171$
	inc	(_timer0_sloc0_1_0 + 1)
00171$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00172$
	inc	r3
00172$:
;	src/main.c:270: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	_timer0_sloc1_1_0,a
	inc	_timer0_sloc0_1_0
	clr	a
	cjne	a,_timer0_sloc0_1_0,00173$
	inc	(_timer0_sloc0_1_0 + 1)
00173$:
	mov	dpl,r2
	mov	dph,r3
	mov	a,_timer0_sloc1_1_0
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00174$
	inc	r3
00174$:
;	src/main.c:271: *m_ptr++ = *f_ptr++;
	mov	dpl,_timer0_sloc0_1_0
	mov	dph,(_timer0_sloc0_1_0 + 1)
	movx	a,@dptr
	mov	r1,a
	mov	a,#0x01
	add	a, _timer0_sloc0_1_0
	mov	r4,a
	clr	a
	addc	a, (_timer0_sloc0_1_0 + 1)
	mov	r5,a
	mov	dpl,r2
	mov	dph,r3
	mov	a,r1
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00175$
	inc	r3
00175$:
;	src/main.c:273: *m_ptr++ = line_counter | 0x08;
	mov	a,#0x08
	orl	a,_timer0_line_counter_10000_31
	mov	dpl,r2
	mov	dph,r3
	movx	@dptr,a
	inc	r2
	cjne	r2,#0x00,00176$
	inc	r3
00176$:
;	src/main.c:275: m_ptr += 7;
	mov	a,#0x07
	add	a, r2
	mov	r6,a
	clr	a
	addc	a, r3
	mov	r7,a
;	src/main.c:261: for (uint8_t i = 0; i < 4; i++) {
	inc	_timer0_i_20001_35
	ljmp	00109$
00102$:
;	src/main.c:278: line_counter++;
	inc	_timer0_line_counter_10000_31
;	src/main.c:279: if (line_counter == 8) {
	mov	a,#0x08
	cjne	a,_timer0_line_counter_10000_31,00104$
;	src/main.c:280: line_counter = 0;
	mov	_timer0_line_counter_10000_31,#0x00
;	src/main.c:281: tick_counter++;
	inc	_tick_counter
	clr	a
	cjne	a,_tick_counter,00179$
	inc	(_tick_counter + 1)
00179$:
00104$:
;	src/main.c:283: P3_5 = 0;
;	assignBit
	clr	_P3_5
;	src/main.c:284: }
	pop	psw
	pop	dph
	pop	dpl
	pop	b
	pop	acc
	reti