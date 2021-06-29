#include "data.h"
#include<fstream>
#include<string>
#include<iostream>
#include<sstream>

void readData( std::vector<std::vector<int>>& lines){

    std::ifstream infile ("2kD100T10.data");
    
	std::string line ;
	
	while ( getline(infile, line))
	{
		std::vector<int> tokens ;
		std::stringstream ls(line) ;
		int token = 0,i=1 ;
 
		while ( ls >> token ){
			if(i){
				i--;
				continue;
			}
			tokens.push_back(token) ;
		}
 
		if ( tokens.size() )		
			lines.emplace_back( move(tokens)) ;
	}

}


void readTimeAllowance(std::vector<double>& time_allowance, const int& speed){

	std::ifstream infile ("poisson.ignore");
	
	std::string line;
	int count=2000;

	while(std::getline(infile, line) && count--){
		std::stringstream ls(line);
		double token;
		ls >> token; 
		time_allowance.emplace_back(token/speed);
	}
}
