#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
using namespace std;

const int ROWS = 20;
const int COLS = 20;
const int DOODLE_BUGS = 5;
const int ANTS = 100;
const int DOODLE_STARVE_TIMER = 3;
const int DOODLE_BREED = 8;
const int ANT_BREED = 3;
const string DOODLE = "X";
const string ANT = "o";


class Position{
    int row;
    int col;
public:
    void set_row(int new_row){row = new_row;}
    int get_row() const{return row;}
    void set_col(int new_col){col = new_col;}
    int get_col() const{return col;}
    Position(int new_row = 0, int new_col = 0) : row(new_row), col(new_col){}
    bool operator ==(const Position& rhs);
};

class Organism{
protected:
    string identity;
    int precedence;
    int time_breed;
    Position position;
public:
    virtual void move();
    virtual void breed(vector<Organism> &organisms);
    Position get_position() const{return position;}
    string get_identity() const{ return identity;}
    Organism(string new_identity = "-", int new_precedence = 0, int new_time_breed = 3, int rows = 0, int cols = 0)
            : identity(new_identity), precedence(new_precedence), time_breed(new_time_breed), position(rows,cols){}
    Organism(int rows, int cols);
    int get_breed_timer(){return time_breed;}
    friend ostream& operator <<(ostream& outs, const Organism& lhs);
};

class Ant; //have to declare because referenced ants in Doodlebug

class Doodlebug : public Organism{
    int starve_timer;
    bool ate;
public:
    Doodlebug(string new_identity = DOODLE, int new_precedence = 1, int new_time_breed = DOODLE_BREED, int rows = 0, int cols = 0, int new_starve_timer = DOODLE_STARVE_TIMER, bool has_eaten = false)
            : Organism(new_identity, new_precedence, new_time_breed, rows, cols), starve_timer(new_starve_timer), ate(has_eaten){}
    Doodlebug(int rows, int cols);
    void move(vector<Doodlebug> &doodle, vector<Ant> ants);
    void breed(vector<Doodlebug> &doodle, vector<Ant> ants);
    int get_starve_timer() const{return starve_timer;}
    void set_starve_timer(int timer){starve_timer = timer;};
    void set_ate(bool has_eaten){ate = has_eaten;}
    bool get_ate() const {return ate;}
    bool to_eat_or_not(const vector<Ant> ants);
    Position get_position() const{return position;}
    void eat(vector<Ant> &ants);
};

class Ant : public Organism{
    bool eaten;
public:
    Ant(string new_identity = ANT, int new_precedence = 0, int new_time_breed = ANT_BREED, int rows = 0, int cols = 0, bool has_been_eaten = false)
            : Organism(new_identity, new_precedence, new_time_breed, rows, cols), eaten(has_been_eaten){}
    Ant(int rows, int cols);
    void move(vector<Ant> &ants, vector<Doodlebug> doodle);
    void breed(vector<Ant> &ants, vector<Doodlebug> doodle);
    void set_eaten(bool has_been_eaten){ eaten = has_been_eaten;}
    bool get_eaten() const{return eaten;}
    Position get_position() const{return position;}
};

class Board{
    int total_rows;
    int total_cols;
    int time_step;
public:
    void generate_board(vector<Doodlebug> p1, vector<Ant> p2);
    Board(int new_row = 0, int new_col = 0, int new_time = 0)
            : total_rows(new_row), total_cols(new_col), time_step(new_time){}
    int get_time_step() const{return time_step;}
};


//non-class prototypes
bool position_repeated(vector<Position> positions, Position potential_position);
vector<Doodlebug> death_from_starvation(vector<Doodlebug> doodle);
vector<Ant> survived_eating(const vector<Ant> ants);
bool matching_pos_doodle(vector<Doodlebug> orgs, Position temporary, int& index);
bool matching_pos_ant(vector<Ant> orgs, Position temporary, int& index);
void generate_doodle_bugs(vector<Doodlebug> &doodle, vector<Position> &positions, int population);
void generate_ants(vector<Ant> &ants, vector<Position> &positions, int population);
void game_start(vector<Doodlebug> &doodle, vector<Ant> &ants, Board layout);
bool ant_position_occupied(vector<Ant> ants, Position new_position);
bool doodle_position_occupied(vector<Doodlebug> doodle, Position new_position);
void print_intro();



int main() {
    srand(time(0));
    vector<Position> starting_positions;
    vector<Doodlebug> doodle;
    vector<Ant> ants;

    //to generate the initial doodle_bugs and ants at random spaces
    generate_doodle_bugs(doodle, starting_positions, DOODLE_BUGS);
    generate_ants(ants, starting_positions, ANTS);

    print_intro();
    cout << "Here is our initial board\n";
    cout << endl;
    //to initialize the board
    Board environment(ROWS, COLS, 0);
    environment.generate_board(doodle, ants);


    //Game will start!
    game_start(doodle, ants, environment);

    return 0;
}



Organism::Organism(int rows, int cols) {
    this->position = (rows, cols);
}

Doodlebug::Doodlebug(int rows, int cols) {
    identity = DOODLE;
    precedence = 1;
    time_breed = DOODLE_BREED;
    starve_timer = DOODLE_STARVE_TIMER;
    ate = false;
    this->position = Position(rows, cols);
}
Ant::Ant(int rows, int cols) {
    identity = ANT;
    precedence = 0;
    time_breed = ANT_BREED;
    eaten = false;
    this->position = Position(rows, cols);
}

void game_start(vector<Doodlebug> &doodle, vector<Ant> &ants, Board layout){
    bool stop = false;
    string input;
    while(stop == false){
        cout << "Press ENTER for next time-step\n";
        getline(cin, input);
        if(input != ""){
            stop = true;
        }
        cout << endl;

        for(int i = 0; i < doodle.size(); i++) {
            //if there are ants nearby we are going to eat else we move
            if (doodle[i].to_eat_or_not(ants) == true) {
                doodle[i].eat(ants);
                //After an eating has taken place it is important to update the vector for remaining ants who have survived
                ants = survived_eating(ants);
            }

            else{
                doodle[i].move(doodle, ants);
            }
        }

        //resetting ate boolean after all doodlebugs have moved
        for(int i = 0; i < doodle.size(); i++){
            doodle[i].set_ate(false);
        }

        //Breed afterwards if the breed-timer is met
        for(int i = 0; i < doodle.size(); i++){
            if(doodle[i].get_breed_timer() == 0){
                doodle[i].breed(doodle, ants);
            }
        }


        //now after the Doodlebugs, it is time for the Ants to move
        for(int i = 0; i < ants.size(); i++){
            ants[i].move(ants, doodle);
        }
        for(int i = 0; i < ants.size(); i++){
            if(ants[i].get_breed_timer() == 0){
                ants[i].breed(ants, doodle);
            }
        }

        //At the end check if any doodlebugs have died from starvation and update the vector
        doodle = death_from_starvation(doodle);

        cout << "Summary of time step " << layout.get_time_step() << "\n";
        cout << "--------------------------\n";
        cout << "Doodlebug Count: " << doodle.size() << endl;
        cout << "Ant Count: " << ants.size() << endl;

        //Generate board to showcase all that occurred in this time step
        layout.generate_board(doodle, ants);

        //Exit conditions for extinction of species were extinction to occur
        if(doodle.size() == 0){
            cout << "Doodle Bugs have gone extinct, game has ended\n";
            stop = true;
        }
        if(ants.size() == 0){
            cout << "Ants have gone extinct, game has ended\n";
            stop = true;
        }
    }
}

void generate_doodle_bugs(vector<Doodlebug> &doodle, vector<Position> &positions, int population){
    int bugs = 0;
    while(bugs < population){
        int random_row = rand() % ROWS;
        int random_col = rand() % COLS;
        Position potential_position(random_row, random_col);
        if(position_repeated(positions, potential_position) == false){
            Doodlebug temp(DOODLE, 1, DOODLE_BREED, random_row, random_col, DOODLE_STARVE_TIMER, false);
            doodle.push_back(temp);
            positions.push_back(potential_position);
            bugs += 1;
        }
    }
}

void generate_ants(vector<Ant> &ants, vector<Position> &positions, int population){
    int bugs = 0;
    while(bugs < population){
        int random_row = rand() % ROWS;
        int random_col = rand() % COLS;
        Position potential_position(random_row, random_col);
        if(position_repeated(positions, potential_position) == false){
            Ant temp(ANT, 0, ANT_BREED, random_row, random_col, false);
            ants.push_back(temp);
            positions.push_back(potential_position);
            bugs += 1;
        }
    }
}

//Function used initially upon doodlebug and ant initialization, to prevent same position spwan
bool position_repeated(vector<Position> positions, Position potential_position){
    bool repeat = false;
    for(int i = 0; i < positions.size(); i++){
        if(potential_position == positions[i]){
            repeat = true;
        }
    }
    return repeat;
//    //if the above doesn't work, simply add algorithm in directive and uncomment below
//    if(count(positions.begin(), positions.end(), potential_position)){
//        return true;
//    }
//    else{
//        return false;
//    }
}

//Returns a vector for the doodlebugs who have not died from starvation
vector<Doodlebug> death_from_starvation(vector<Doodlebug> doodle){
    vector<Doodlebug> new_doodle;
    for(int i = 0; i < doodle.size(); i++){
        if(doodle[i].get_starve_timer() > 0){
            new_doodle.push_back(doodle[i]);
        }
    }
    return new_doodle;
}

//Returns a vector for ants who have not been eaten by Doodlebugs
vector<Ant> survived_eating(const vector<Ant> ants){
    vector<Ant> survived;
    for(int i = 0; i < ants.size(); i++){
        if(ants[i].get_eaten() == false){
            survived.push_back(ants[i]);
        }
    }
    return survived;
}

//Overloaded == operator as positions were often compared
bool Position::operator ==(const Position& rhs){
    return((this->row == rhs.row) && (this->col == rhs.col));
}

//Initial virtual move function for organisms not utilized in code though as re-defined for ants and Doodlebug
void Organism::move(){
    int movement = rand() % 4;
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    if(movement == 0){ //MOving up
        if(curr_row != 0){
            position.set_row(curr_row - 1);
        }
        else{
            position.set_row(curr_row);
        }
    }
    else if(movement == 1){ //MOving right
        if(curr_col != (COLS - 1)){
            position.set_col(curr_col + 1);
        }
        else{
            position.set_col(curr_col);
        }
    }
    else if(movement == 2){ //MOving down
        if(curr_row != (ROWS - 1)){
            position.set_row(curr_row + 1);
        }
        else{
            position.set_row(curr_row);
        }
    }
    else if(movement == 3){ //MOving Left
        if(curr_col != 0){
            position.set_col(curr_col - 1);
        }
        else{
            position.set_col(curr_col);
        }
    }
    this->time_breed -= 1;
}

//Virtual Breed Function for Organisms if Breeding were to occur randomly, which is not the case for Ants and Doodlebugs, so
//redefined for each derived class so Breeding is not random but occurs if an adjacent spot is open
//Good to reference though if need an Organism to Breed Randomly much like the move fucntion.
void Organism::breed(vector<Organism> &organisms){
    int breed = rand() % 4;
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    Organism potential_organism;
    if(breed == 0){ //breed up
        if(curr_row != 0){
            potential_organism = Organism(curr_row - 1, curr_col);
            organisms.push_back(potential_organism);
        }
    }
    else if(breed == 1){ //breed right
        if(curr_col != (COLS - 1)){
            potential_organism = Organism(curr_row, curr_col + 1);
            organisms.push_back(potential_organism);
        }
    }
    else if(breed == 2){ //breed down
        if(curr_row != (ROWS - 1)){
            potential_organism = Organism(curr_row + 1, curr_col);
            organisms.push_back(potential_organism);
        }
    }
    else if(breed == 3){ //Breed Left
        if(curr_col != 0){
            potential_organism = Organism(curr_row, curr_col - 1);
            organisms.push_back(potential_organism);
        }
    }
}

bool doodle_position_occupied(vector<Doodlebug> doodle, Position new_position){
    bool occupied = false;
    for(int i = 0; i < doodle.size(); i++){
        if(doodle[i].get_position() == new_position){
            occupied = true;
        }
    }
    return occupied;
}

bool ant_position_occupied(vector<Ant> ants, Position new_position){
    bool occupied = false;
    for(int i = 0; i < ants.size(); i++){
        if(ants[i].get_position() == new_position){
            occupied = true;
        }
    }
    return occupied;
}

//Move for Doodlebug
void Doodlebug::move(vector<Doodlebug> &doodle, vector<Ant> ants){
    int movement = rand() % 4;
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    if(movement == 0 && this->ate == false){ //MOving up
        Position potential(curr_row - 1, curr_col);
        if(curr_row != 0 && !doodle_position_occupied(doodle, potential)){
            position.set_row(curr_row - 1);
        }
        this->starve_timer -= 1;
        this->time_breed -= 1;
    }
    else if(movement == 1 && this->ate == false){ //MOving right
        Position potential(curr_row, curr_col + 1);
        if(curr_col != (COLS - 1) && !doodle_position_occupied(doodle, potential)){
            position.set_col(curr_col + 1);
        }
        this->starve_timer -= 1;
        this->time_breed -= 1;
    }
    else if(movement == 2 && this->ate == false){ //MOving down
        Position potential(curr_row + 1, curr_col);
        if(curr_row != (ROWS - 1) && !doodle_position_occupied(doodle, potential)){
            position.set_row(curr_row + 1);
        }
        this->starve_timer -= 1;
        this->time_breed -= 1;
    }
    else if(movement == 3 && this->ate == false){ //MOving Left
        Position potential(curr_row, curr_col - 1);
        if(curr_col != 0 && !doodle_position_occupied(doodle, potential)){
            position.set_col(curr_col - 1);
        }
        this->starve_timer -= 1;
        this->time_breed -= 1;
    }
}

//Move for Ant
void Ant::move(vector<Ant> &ants, vector<Doodlebug> doodle){
    int movement = rand() % 4;
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    if(movement == 0){
        Position potential(curr_row - 1, curr_col);
        if(curr_row != 0 && !doodle_position_occupied(doodle, potential) && !ant_position_occupied(ants, potential)){
            position.set_row(curr_row - 1);
        }
    }
    else if(movement == 1){
        Position potential(curr_row, curr_col + 1);
        if(curr_col != (COLS - 1) && !doodle_position_occupied(doodle, position) && !ant_position_occupied(ants, position)){
            position.set_col(curr_col + 1);
        }
    }
    else if(movement == 2){
        Position potential(curr_row + 1, curr_col);
        if(curr_row != (ROWS - 1) && !doodle_position_occupied(doodle, position) && !ant_position_occupied(ants, position)){
            position.set_row(curr_row + 1);
        }
    }
    else if(movement == 3){
        Position potential(curr_row, curr_col - 1);
        if(curr_col != 0 && !doodle_position_occupied(doodle, potential) && !ant_position_occupied(ants, potential)){
            position.set_col(curr_col - 1);
        }
    }
    this->time_breed -= 1;
}


void Doodlebug::breed(vector<Doodlebug> &doodle, vector<Ant> ants){
        bool has_bred = false;
        int curr_row = this->position.get_row();
        int curr_col = this->position.get_col();
        Position breed_up(curr_row - 1, curr_col);
        Position breed_right(curr_row, curr_col + 1);
        Position breed_down(curr_row + 1, curr_col);
        Position breed_left(curr_row, curr_col - 1);
        vector<Doodlebug> bred;
            if (curr_row != 0 && doodle_position_occupied(doodle, breed_up) == false && ant_position_occupied(ants, breed_up)== false && has_bred == false) {
                Doodlebug potential_doodle_bug(curr_row - 1, curr_col);
                bred.push_back(potential_doodle_bug);
                this->time_breed = DOODLE_BREED;
                has_bred = true;
            }

            else if(curr_col != (COLS - 1) && doodle_position_occupied(doodle, breed_right) == false && ant_position_occupied(ants, breed_right) == false && has_bred == false) {
                Doodlebug potential_doodle_bug(DOODLE, 1, DOODLE_BREED, curr_row, curr_col + 1, DOODLE_STARVE_TIMER, false);
                bred.push_back(potential_doodle_bug);
                this->time_breed = DOODLE_BREED;
                has_bred = true;
            }

            else if(curr_row != (ROWS - 1) && doodle_position_occupied(doodle, breed_down) == false && ant_position_occupied(ants, breed_down) == false && has_bred == false) {
                Doodlebug potential_doodle_bug(DOODLE, 1, DOODLE_BREED, curr_row + 1, curr_col, DOODLE_STARVE_TIMER, false);
                bred.push_back(potential_doodle_bug);
                this->time_breed = DOODLE_BREED;
                has_bred = true;
            }

            else if(curr_col != 0 && doodle_position_occupied(doodle, breed_left) == false && ant_position_occupied(ants, breed_left) == false && has_bred == false) {
                Doodlebug potential_doodle_bug(DOODLE, 1, DOODLE_BREED, curr_row, curr_col - 1, DOODLE_STARVE_TIMER, false);
                bred.push_back(potential_doodle_bug);
                this->time_breed = DOODLE_BREED;
                has_bred = true;
            }
                //if still couldn't breed, reset back to one so breeding can attempt to occur next turn
            else if(has_bred == false){
                this->time_breed = 1;
            }

        for(int i = 0; i < bred.size(); i++){
            doodle.push_back(bred[i]);
        }
}

void Ant::breed(vector<Ant> &ants, vector<Doodlebug> doodle){
        bool has_bred = false;
        int curr_row = this->position.get_row();
        int curr_col = this->position.get_col();
        Position breed_up(curr_row - 1, curr_col);
        Position breed_right(curr_row, curr_col + 1);
        Position breed_down(curr_row + 1, curr_col);
        Position breed_left(curr_row, curr_col - 1);
        vector<Ant> bred;
        if (curr_row != 0 && doodle_position_occupied(doodle, breed_up) == false && ant_position_occupied(ants, breed_up) == false && has_bred == false) {
            Ant potential_ant(curr_row - 1, curr_col);
            bred.push_back(potential_ant);
            this->time_breed = ANT_BREED;
            has_bred = true;
        }

        else if(curr_col != (COLS - 1) && doodle_position_occupied(doodle, breed_right) == false && ant_position_occupied(ants, breed_right) == false && has_bred == false) {
            Ant potential_ant(curr_row, curr_col + 1);
            bred.push_back(potential_ant);
            this->time_breed = ANT_BREED;
            has_bred = true;
        }

        else if(curr_row != (ROWS - 1) && doodle_position_occupied(doodle, breed_down) == false && ant_position_occupied(ants, breed_down) == false && has_bred == false) {
            Ant potential_ant(curr_row + 1, curr_col);
            bred.push_back(potential_ant);
            this->time_breed = ANT_BREED;
            has_bred = true;
        }

        else if(curr_col != 0 && doodle_position_occupied(doodle, breed_left) == false && ant_position_occupied(ants, breed_left) == false) {
            Ant potential_ant(curr_row, curr_col - 1);
            bred.push_back(potential_ant);
            this->time_breed = ANT_BREED;
            has_bred = true;
        }
            //if still couldn't breed, reset back to one so breeding can attempt to occur next turn
        else if(has_bred == false){
            this->time_breed = 1;
        }

    for(int i = 0; i < bred.size(); i++){
        ants.push_back(bred[i]);
    }
}

//Check adjacent cells(up right down left) to see if ants are there to eat
bool Doodlebug::to_eat_or_not(const vector<Ant> ants){
    bool eating = false;
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    Position eat_up(curr_row - 1, curr_col);
    Position eat_right(curr_row, curr_col + 1);
    Position eat_down(curr_row + 1, curr_col);
    Position eat_left(curr_row, curr_col - 1);

    for(int i = 0; i < ants.size(); i++){
        if((ants[i].get_position() == eat_up && ants[i].get_eaten() == false) || (ants[i].get_position() == eat_right && ants[i].get_eaten() == false) ||
           (ants[i].get_position() == eat_down && ants[i].get_eaten() == false) || (ants[i].get_position() == eat_left && ants[i].get_eaten() == false)){
            eating = true;
        }
    }
    return eating;
}

//Eat function Doodlebug will move to cell with ant and boolean for ant (eaten) will change to be true
//Vector for ant will be then updated afterwards to remove eaten ants from board;
void Doodlebug::eat(vector<Ant> &ants){
    int curr_row = this -> position.get_row();
    int curr_col = this -> position.get_col();
    Position eat_up(curr_row - 1, curr_col);
    Position eat_right(curr_row, curr_col + 1);
    Position eat_down(curr_row + 1, curr_col);
    Position eat_left(curr_row, curr_col - 1);

    for(int i = 0; i < ants.size(); i++){
        if(ants[i].get_position() == eat_up && ants[i].get_eaten() == false && this->get_ate() == false){ //if the ant in that position is not eaten
            ants[i].set_eaten(true); //we will eat it and set bool to true
            position.set_row(curr_row - 1);
            this->set_ate(true);
            this->set_starve_timer(3);  //replenish it to be 3 turns before this doodlebug starves
            this->time_breed -= 1;
        }
        else if(ants[i].get_position() == eat_right && ants[i].get_eaten() == false && this->get_ate() == false){
            ants[i].set_eaten(true);
            position.set_col(curr_col + 1);
            this->set_ate(true);
            this->set_starve_timer(3);
            this->time_breed -= 1;
        }
        else if(ants[i].get_position() == eat_down && ants[i].get_eaten() == false && this->get_ate() == false){
            ants[i].set_eaten(true);
            position.set_row(curr_row + 1);
            this->set_ate(true);
            this->set_starve_timer(3);
            this->time_breed -= 1;
        }
        else if(ants[i].get_position() == eat_left && ants[i].get_eaten() == false && this->get_ate() == false){
            ants[i].set_eaten(true);
            position.set_col(curr_col - 1);
            this->set_ate(true);
            this->set_starve_timer(3);
            this->time_breed -= 1;
        }
    }
}

bool matching_pos_doodle(vector<Doodlebug> orgs, Position temporary, int& index){
    bool found = false;
    for(int i = 0; i < orgs.size(); i++){
        if(orgs[i].get_position() == temporary){
            index = i;
            found = true;
        }
    }
    return found;
}

bool matching_pos_ant(vector<Ant> orgs, Position temporary, int& index){
    bool found = false;
    for(int i = 0; i < orgs.size(); i++){
        if(orgs[i].get_position() == temporary){
            index = i;
            found = true;
        }
    }
    return found;
}

ostream& operator <<(ostream& outs, const Organism& rhs){
    outs << rhs.identity;
    return outs;
}

void Board::generate_board(vector<Doodlebug> p1, vector<Ant> p2){
    int index;
    for(int i = 0; i < total_rows; i++){
        for(int j = 0; j < total_cols; j++){
            Position temporary = Position(i,j);
            if(matching_pos_doodle(p1, temporary, index)){
                cout << p1[index].get_identity() << ' ';
            }
            else if(matching_pos_ant(p2, temporary, index)){
                cout << p2[index].get_identity() << ' ';
            }
            else{
                cout << "- ";
            }
        }
        cout << endl;
    }
    this->time_step += 1;
}

void print_intro(){
    cout << "Welcome to the Doodle Bug/Ant simulation.\n";
    cout << "A 20 by 20 Board(Matrix) will be comprised of Doodlebugs and Ants spawned randomly upon world Intialization\n";
    cout << "Each time step will be generated upon user input of ENTER\n";
}