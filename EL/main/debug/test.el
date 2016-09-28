if (0 < x) {
  fact = 1
  while (x < 10) {
    fact = fact * x;
    x = x + 1;
  }

  write fact
}

read x;

while (y < x) {
	write y
}

do {
	x = x + 1;
} while(x < 10);

if (x) { read x; }

function printf(format, arguments) {
	write format; 
	write arguments;
} 

function stop() {
}

stop();