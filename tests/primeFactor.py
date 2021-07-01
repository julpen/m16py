def primeFactor(n):
    putval(n)
    put('=')
    if (n < 0):
        putval(-1)
        put('*')
        n = -n
    if (n == 1):
        print(1)
        return 0
    f = 2
    while f <= n:
        if (n % f) == 0:
            n = n/f
            putval(f)
            if n != 1:
                put('*')
        else:
            f = f+1
    put('\n')
    return 0
