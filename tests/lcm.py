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