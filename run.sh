run() {
	k=$1
	mkdir "k=$k"

	nr_trials=0
	until [ ! $nr_trials -lt $2 ]
	do 
		echo "\n$nr_trials"

		cd server 
		./scdptopk -r 0 -k $k &
		cd ../client 
		sleep 0.1
		time ./scdptopk -r 1 -k $k

		cd ..
		pwd
		cat server/Runtime.out > "k=$k/$nr_trials.srv.out"
		cat client/Runtime.out > "k=$k/$nr_trials.cli.out"

		nr_trials=`expr $nr_trials + 1 `
	done
}

k_min=1
k_max=51
trials=5

k=$k_min

until [ ! $k -lt $k_max ]
do
	echo "\nk=$k"
	run $k $trials
	k=`expr $k + 1`
done
