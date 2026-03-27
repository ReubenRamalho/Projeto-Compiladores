fun pot(base, exp) {
    var res = 1;
    
    if exp == 0 {
        res = 1;
    } else {
        res = base * pot(base, exp - 1);
    }
    
    return res;
}

main {
    return pot(2, 5);
}