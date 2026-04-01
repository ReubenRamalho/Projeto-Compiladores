var dividendo = 27;
var divisor = 4;
var quociente = 0;

main {
    while dividendo >= divisor {
        dividendo = dividendo - divisor;
        quociente = quociente + 1;
    }
    return quociente;
}