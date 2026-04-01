fun mdc(a, b) {
    var temp = 0;
    
    while a != b {
        if a >= b {
            a = a - b;
        } else {
            b = b - a;
        }
    }
    
    return a;
}

main {
    return mdc(48, 18);
}