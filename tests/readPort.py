OCR0 = 0x5C # Address of register

val = 1000

while val > 0:
    set(OCR0, get(OCR0)-1)
    print(get(OCR0))
    val = val - 1
