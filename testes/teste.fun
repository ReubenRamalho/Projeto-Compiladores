fun fat(n) {
    var res = 1;
    if n < 2 {
        res = 1;
    } else {
        res = n * fat(n - 1);
    }
    return res;
}

main {
    return fat(5);
}