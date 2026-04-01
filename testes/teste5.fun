var base = 999; 

fun pow(base, expoente) {
    var res = 1;
    
    while expoente != 0 {
        res = res * base;
        expoente = expoente - 1;
    }
    
    return res;
}

main {
    return pow(2, 5); 
}