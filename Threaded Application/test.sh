./build.sh
timeout 50s ./prime > primes_found.txt
diff primes.txt primes_found.txt > difference.txt
