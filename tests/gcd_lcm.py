def gcd(a,b):
    if a == 0:
        return 0
    else:
        while b != 0:
            if a > b:
                a = a - b
            else:
                b = b - a
    return a

def lcm(a,b):
    in1 = a
    in2 = b
    if a == 0:
        return 0
    else:
        while b != a:
            if a > b:
                b = b + in2
            else:
                a = a + in1

        return a


a = 36
b = 51

lcm(a,b) == a/gcd(a,b)*b