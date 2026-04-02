var x = true;
var y = false;

fun teste(a, b) {
 var z = not a or b;
 return z and true;
}

main {
 x = not y;
 y = teste(x, false) or (x and true);
 return y;
}