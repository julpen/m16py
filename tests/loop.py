def loopTest(a,b,c,d):
    it = 0
    tmp1 = b
    tmp2 = c
    tmp3 = d
    while a > 0:
        b = tmp1
        while b > 0:
            c = tmp2
            while c > 0:
                d = tmp3
                while d > 0:
                    d = d-1
                    it = it+1
                c = c-1
            b = b-1
        a = a-1
    return it
