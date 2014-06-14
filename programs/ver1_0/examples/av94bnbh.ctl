;
; M6811DIS Control File for:
;
;  '94 Astro Van computer code: BNBH
;

input AV94BNBH.BIN
output AV94BNBH.DIS

load 4000

addresses
ascii

label ffd6 scivect
label ffd8 spivect
label ffda paievect
label ffdc paovect
label ffde tovfvect
label ffe0 ti4o5vect
label ffe2 to4vect
label ffe4 to3vect
label ffe6 to2vect
label ffe8 to1vect
label ffea ti3vect
label ffec ti2vect
label ffee ti1vect
label fff0 rtivect
label fff2 irqvect
label fff4 xirqvect
label fff6 swivect
label fff8 ilopvect
label fffa copvect
label fffc cmonvect
label fffe rstvect

indirect ffd6 scirtn
indirect ffd8 spirtn
indirect ffda paiertn
indirect ffdc paortn
indirect ffde tovfrtn
indirect ffe0 ti4o5rtn
indirect ffe2 to4rtn
indirect ffe4 to3rtn
indirect ffe6 to2rtn
indirect ffe8 to1rtn
indirect ffea ti3rtn
indirect ffec ti2rtn
indirect ffee ti1rtn
indirect fff0 rtirtn
indirect fff2 irqrtn
indirect fff4 xirqrtn
indirect fff6 swirtn
indirect fff8 iloprtn
indirect fffa coprtn
indirect fffc cmonrtn
indirect fffe reset

entry 7C0B
entry 7C12
entry 7C1C
entry 7C22
entry 7C35
entry 7C6B
entry 7C7C
entry 7C83
entry 7C9C
entry 7CA0
entry 7CAA
entry 7CAE
entry 7CBE
entry 7CC2
entry 7CCC
entry 7CDD
