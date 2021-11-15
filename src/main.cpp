#include "THC-chess/src/thc.h"
#include "UCI/UCI.h"
#include <iostream>
#include <sstream>
#include <memory>
#define DEBUG 1

void display_position( thc::ChessRules &cr, const std::string &description )
{
    std::string fen = cr.ForsythPublish();
    std::string s = cr.ToDebugStr();
    printf( "%s\n", description.c_str() );
    printf( "FEN (Forsyth Edwards Notation) = %s\n", fen.c_str() );
    printf( "Position = %s\n", s.c_str() );
}

int main()
{
    std::shared_ptr<thc::ChessRules> cr = std::shared_ptr<thc::ChessRules>(new thc::ChessRules());
    thc::Move mv;
    UCI_opt PP_UCI_opt;

    /* TODO:
        MCTS declare here.
    */
    
    std::string line;
	std::string token;

    while (std::getline(std::cin, line)){
        std::istringstream is(line);
        token.clear();
		is >> std::skipws >> token;

		if (token == "uci")
		{
			std::cout << "id name PP project" << std::endl;
			std::cout << "id author " << std::endl;
			PP_UCI_opt.print_UCI_opt();
			std::cout << "uciok" << std::endl;
		}
		else if (token == "isready"){
			std::cout << "readyok" << std::endl;
		}
		else if (token == "setoption") {
            std::string name, value;
            while (is >> token){
                if (token == "name") is >> name;
                else if(token == "value") is >> value;
            }
            PP_UCI_opt.set_UCI_opt(name, value);
        }
		else if (token == "color"){
            const char *color;
            color = cr->WhiteToPlay() ? "white" : "black";
            std::cout << "colorToPlay: " << color << std::endl;
        }
		else if (token == "ucinewgame"){
		    cr = std::shared_ptr<thc::ChessRules>(new thc::ChessRules());
        }
		else if (token == "position"){
            std::string token, fen;
            is >> token;
            if (token == "startpos"){
                cr = std::shared_ptr<thc::ChessRules>(new thc::ChessRules());
            }
            else if (token == "fen"){
                while (is >> token && token != "moves")
			        fen += token + " ";
            }
            else{
                std::cout << "you should input fen format string.\n";
                continue;
            }

            cr->Forsyth(fen.c_str());

            // Parse move list (if any)
            while (is >> token){
                if (token != "moves"){
                    mv.TerseIn(cr.get(), token.c_str());
                    cr->PlayMove(mv);
                }
            }
#ifdef DEBUG
            display_position(*cr, "");
#endif 
        }
		else if (token == "print"){//for debug, not in UCI standard
            display_position(*cr, "Current board");
        }
		else if (token == "printOptions"){//for debug, not in UCI standard
            PP_UCI_opt.print_UCI_opt();
        }
		else if (token == "go"){
            struct UCI_go_opt go_opt;
            go_opt.infinite = false;
			while (is >> token){
				if (token == "wtime")           is >> go_opt.wtime;
				else if (token == "btime")      is >> go_opt.btime;
				else if (token == "winc")       is >> go_opt.winc;
				else if (token == "binc")       is >> go_opt.binc;
				else if (token == "movestogo")  is >> go_opt.movestogo;
                else if (token == "depth")      is >> go_opt.depth;
                else if (token == "nodes")      is >> go_opt.nodes;
                else if (token == "mate")       is >> go_opt.mate;
                else if (token == "movetime")   is >> go_opt.movetime;
                else if (token == "infinite")   go_opt.infinite = true;
			}
            /* TODO
                mv = MCTS.run(go_opt, cr);
            */
            std::cout << "bestmove " << mv.TerseOut() << std::endl;
            cr->PlayMove(mv);
#ifdef DEBUG
            display_position(*cr, "");
#endif 
		}
		else if (token == "quit"){
			std::cout << "Terminating.." << std::endl;
			break;
		}
		else // Command not handled
			std::cout << "what?" << std::endl;
    }
    return 0;
}