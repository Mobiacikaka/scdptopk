add_library(dataset dataset.cpp)
add_library(party party.cpp)
add_library(server server.cpp)
add_library(client client.cpp)

add_executable(scdptopk main.cpp)

target_link_libraries(server party dataset)
target_link_libraries(client party dataset)

target_link_libraries(
	scdptopk

    dataset
    party
    server
    client

	aby 
	otextension
	encrypto_utils 
	crypto
	relic_s
	pthread
	ssl
	gmp
	gmpxx
	boost_system
)
