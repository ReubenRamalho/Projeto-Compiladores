fun max_tres(a, b, c) {
    var maior = 0;
    
    if a >= b {
        if a >= c {
            maior = a;
        } else {
            maior = c;
        }
    } else {
        if b >= c {
            maior = b;
        } else {
            maior = c;
        }
    }
    
    return maior;
}

main {
    return max_tres(15, 42, 8);
}