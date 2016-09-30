x = -+-(-2);
if (x > - 2) {
  fact = 1
  while (x < 10) {
    fact = fact * x;
    x = x + 1;
  }
  
  x = x - 5
  write fact
}

read x; write x;

while (y < x) {
	write y
}

do {
	x = x + 1;
} while(x < 10);

if (x) { read x; }

function stop() {
}

function printf(format, argument1, argument2, argument3) {
	write format; 
	write argument1;
	write argument2;
	write argument3;
	
	stop();
} 

printf("%d", 12);
