var acc = 0;

fun formula(a, b, c) {
    var temp = a * b;
    return temp - c;
}

main {
    acc = formula(10, 2, 4);
    return acc;
}