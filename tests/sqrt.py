def sqrt(a):
    x0 = a >> 1
    if x0:
        x1 = (x0 + a/x0) >> 1

        while x1 < x0:
            x0 = x1
            x1 = (x0 + a/x0) >> 1
        return x0
    else:
        return a
