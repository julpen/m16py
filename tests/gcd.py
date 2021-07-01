def minus(a, b):
    return a-b

def gcd(a,b):
    if a == 0:
        return 0
    else:
        while b != 0:
            if a > b:
                a = minus(a,b)
            else:
                b = minus(b,a)
    put('G', 'C', 'D', '=')
    print(a)
    return a

