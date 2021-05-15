if [ ! -f "build/scdptopk" ]; then
	./build.sh
fi

run() {
	k=$1
	mkdir "$k"

	nr_trials=0
	until [ ! $nr_trials -lt $2 ]
	do 
		echo "\n"
		echo "$nr_trials"

		cd server 
		./scdptopk -r 0 -k $k &
		cd ../client 
		sleep 0.1
		time ./scdptopk -r 1 -k $k

		cd ..
		cat server/Selection.out > "$k/$nr_trials.out"
		cat client/Selection.out >> "$k/$nr_trials.out"

		nr_trials=`expr $nr_trials + 1`
		rm server/*.out client/*.out -rf
	done
	
	# cd ..
}

rm exp -rf && mkdir exp && cd exp
mkdir server && mkdir client
cp "../datasets/server.txt" "server/dataset.txt"
cp "../datasets/client.txt" "client/dataset.txt"
cp "../build/scdptopk" "server/"
cp "../build/scdptopk" "client/"

k=1
maxk=101
trials=50

until [ ! $k -lt $maxk ]
do
	echo
	echo k=$k
	run $k $trials
	k=`expr $k + 1`
done
