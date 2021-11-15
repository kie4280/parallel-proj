#ifndef UCI_H
#define UCI_H

#include <string>
#include <vector>

class UCI_opt
{
public:
	UCI_opt();
	void print_UCI_opt();
	int set_UCI_opt(std::string opt_name, std::string value);
private:
	int dump;
};


struct UCI_go_opt
{
	unsigned int wtime;
		//white has x msec left on the clock
	unsigned int btime;
		//black has x msec left on the clock
	unsigned int winc;
		//white increment per move in mseconds if x > 0
	unsigned int binc;
		//black increment per move in mseconds if x > 0
	unsigned int movestogo;
		//there are x moves to the next time control,
	unsigned int depth;
		//search x plies only.
	unsigned int nodes;
		//search x nodes only,
	unsigned int mate;
		//search for a mate in x moves
	unsigned int movetime;
		//search exactly x mseconds
	bool infinite = false;
		//search until the "stop" command. Do not exit the search without being told so in this mode!
};



#endif  // UCI_H
