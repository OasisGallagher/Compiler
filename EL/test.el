read x
write x

if 0 < x then
  fact = 1
  repeat
    fact = fact * x
    x = x + 1
  until x == 0
  write fact
end

repeat
	x = x * 2;
until x >= 10

if x >= 10 then
	write x;
end

if x < 10 then write x end
#
#for i = 1; i < 10; i = i + 1 do
#	write x;
#end
