if (x > 0 - 2) {
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

function printf(format, arguments) {
	write format; 
	write arguments;
	
	stop();
} 

printf("%d", 12);
