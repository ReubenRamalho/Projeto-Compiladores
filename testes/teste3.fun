fun raiz_inteira(n) {
    var i = 1;
    var res = 0;
    
    while i * i <= n {
        res = i;
        i = i + 1;
    }
    
    return res;
}

main {
    return raiz_inteira(50); 
}