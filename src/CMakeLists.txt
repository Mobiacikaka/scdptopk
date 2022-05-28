add_library(dataset dataset.cpp)
add_library(party party.cpp)
add_library(mmh3 MurmurHash3.cpp)

add_executable(scdptopk main.cpp)

target_link_libraries(
	scdptopk

	party
	dataset
	mmh3

	aby 
	otextension
	encrypto_utils 
	crypto
	cryptopp
	bloom
	relic_s
	pthread
	ssl
	gmp
	gmpxx
	boost_system
	stdc++fs
)
