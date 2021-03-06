/* RASSE
 *
 * Kuvaus:
 *
 *
 * Ohjelman kirjoittajat
 * Nimi: Santeri Laurila (274301)
 * Käyttäjätunnus: laurila2
 * E-Mail: santeri.laurila@tuni.fi
 *
 * Nimi: Tuomo Pöllänen (K434889)
 * Käyttäjätunnus: a729530
 * E-Mail: tuomo.pollanen@tuni.fi
 *
 * Huomioita ohjelmasta ja sen toteutuksesta:
 *
 * */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <vector>
#include <algorithm>


// Alustetaan vakiomuuttujat
const char INPUT_FILE_FIELD_DELIMETER = ';';
const size_t LINE_FIELD_INDEX = 0;
const size_t STOP_FIELD_INDEX = 1;
const size_t DIST_FIELD_INDEX = 2;
const size_t VALID_FIELD_COUNT_LOW = 2;
const size_t VALID_FIELD_COUNT_HIGH = 3;

// Tietorakenne pysäkille
struct Stop{
    std::string name;
};
// Tietorakenne pysäkin etäisyydelle
using dist_type = double;
// Tietorakenne linjoille, jotka sisältävät pysäkkejä
using Tramway = std::map<std::string, std::map<dist_type, Stop>>;


// Apufunktio syötteen parimiseksi
std::vector<std::string> split(const std::string &s,
                               const char delimiter,
                               bool ignore_empty = false)
{
    std::vector<std::string> result;
    std::string tmp = s;

    while (tmp.find(delimiter) != std::string::npos)
    {
        std::string new_part = tmp.substr(0, tmp.find(delimiter));
        tmp = tmp.substr(tmp.find(delimiter) + 1, tmp.size());
        if (not(ignore_empty and new_part.empty()))
        {
            result.push_back(new_part);
        }
    }
    if (not(ignore_empty and tmp.empty()))
    {
        result.push_back(tmp);
    }
    return result;
}

// Prints a RASSE
void print_rasse()
{
    std::cout <<
                 "=====//==================//===\n"
                 "  __<<__________________<<__   \n"
                 " | ____ ____ ____ ____ ____ |  \n"
                 " | |  | |  | |  | |  | |  | |  \n"
                 " |_|__|_|__|_|__|_|__|_|__|_|  \n"
                 ".|                  RASSE   |. \n"
                 ":|__________________________|: \n"
                 "___(o)(o)___(o)(o)___(o)(o)____\n"
                 "-------------------------------" << std::endl;
}

// Lukee syötetiedoston rivi kerrallaan, ja lisää rivit vektoriin
// return:   true: tiedoston lukeminen onnistui
//           false: tiedoston lukeminen epäonnistui
bool read_input_file(const std::string filepath, std::vector<std::string>& lines)
{
    // Avataan tiedosto
    std::ifstream input_file_stream;
    input_file_stream.open(filepath);

    // Tulostetaan virheilmoitus ja palataan funktiosta, mikäli tiedosto ei aukea
    if(!input_file_stream.is_open()){
        std::cout << "Error: File cannot be opened." << std::endl;
        return false;
    }

    // Lisätään tiedoston rivit vektoriin
    std::string line;
    while(std::getline(input_file_stream, line)){
        lines.push_back(line);
    }
    input_file_stream.close();
    return true;
}

// Lisää pysäkin Tramway-tietorakenteeseen, halutulle linjalle
// return:   true: lisäys onnistui
//           false: lisäys epäonnistui
bool add_stop(Tramway& rasse_data, const std::string& line_name,
              dist_type distance, const std::string& stop_name){

    // Muuttuja, joka kertoo onko lisättävä pysäkki ainut linjallaan
    bool is_unique  = true;

    // Tarkistetaan, onko linjaa olemassa
    if(rasse_data[line_name].count(distance) != 0){
        is_unique = false;
    }

    // Iteroidaan linja
    for(auto& dist_stop_pair : rasse_data[line_name]){

        // Pysäkkiä ei lisätä, mikäli linjalta löytyy jo saman niminen pysäkki
        if(dist_stop_pair.second.name == stop_name){
            is_unique = false;
            break;
        }
    }

    // Lisätään pysäkki linjalle, mikäli tätä ei jo ole linjalla
    if(is_unique){
        rasse_data[line_name][distance] = Stop{stop_name};
    }

    return is_unique;
}


// Lisää linjan Tramway-tietorakenteeseen
// return:   true: lisäys onnistui
//           false: lisäys epäonnistui
bool add_line(Tramway& rasse_data, const std::string& line_name){

    // Tarkistetaan, onko lisättävä linja jo olemassa
    if(rasse_data.count(line_name) != 0){
        return false;
    }

    // Lisätään tietorakenteeseen haluttu linja
    rasse_data[line_name] = {};
    return true;
}


// Tarkistaa, onko syötetiedoston rivi oikeellisessa muodossa
// return:   true: on
//           false: ei
bool validate_input_fields(Tramway& rasse_data, const std::vector<std::string>& fields){

    // Tarkistetaan onko rivillä hyväksytty määrä erottimella pilkottuja osia
    if(fields.size() < VALID_FIELD_COUNT_LOW && fields.size() > VALID_FIELD_COUNT_HIGH){
        return false;
    }

    // Iteroidaan rivin osat
    for(size_t i = 0; i < fields.size(); ++i){
        std::string field = fields.at(i);

        // Tarkistetaan rivin osat yksi kerrallaan
        if(field == ""
                && i != DIST_FIELD_INDEX
                && rasse_data.count(fields.at(LINE_FIELD_INDEX))){
            return false;
        }
    }
    return true;
}


// Käy syötetiedoston sisällön läpi, ja lisää sen tiedot Tramway-tietorakenteeseen mikäli nämä ovat oikeelliset
// return:   true: läpikäynti onnistui
//           false: läpikäynti epäonnistui
bool parse_input_file(const std::vector<std::string>& rows, Tramway& rasse_data){

    // Iteroidaan rivit sisältävä vektori
    for(const auto& row : rows){
        // Jaetaan rivi osiin erotinmerkin perusteella
        std::vector<std::string> fields = split(row, INPUT_FILE_FIELD_DELIMETER);
        // Tarkistetaan, onko rivin muoto oikeellinen
        if(!validate_input_fields(rasse_data, fields)){
            std::cout << "Error: Invalid format in file." << std::endl;
            return false;
        }

        // Lisätään rivin data muuttujiin
        std::string line_name = fields.at(LINE_FIELD_INDEX);
        std::string stop_name = fields.at(STOP_FIELD_INDEX);
        dist_type distance = 0.0;

        // Jos rivillä on kolme osaa joista etäisyys ei ole tyhjä, otetaan etäisyys talteen
        if(fields.size() == VALID_FIELD_COUNT_HIGH
                && fields.at(DIST_FIELD_INDEX) != ""){

            distance = static_cast<dist_type>(std::stod(fields.at(DIST_FIELD_INDEX)));
        }
         // Jos lisättävää linjaa ei löydy tietorakenteesta, lisätään tämä
        if(rasse_data.count(line_name) == 0){
            if(!add_line(rasse_data, line_name)){
                std::cout << "Error: Stop/line already exists." << std::endl;
            }
        }

        // Lisätään pysäkki
        if(!add_stop(rasse_data, line_name, distance, stop_name)){
            std::cout << "Error: Stop/line already exists." << std::endl;
            return false;
        }
    }

    return true;
}

void print_lines(const Tramway& rasse_data){

    std::cout << "All tramlines in alphabetical order:" << std::endl;

    for(const auto& line : rasse_data){
        std::cout << line.first << std::endl;
    }
}


// Tulostaa kaikki pysäkit aakkosjärjestyksessä
void print_stops(const Tramway& rasse_data){

    std::vector<std::string> stops;
    bool stop_in_vector;

    for(const auto& line : rasse_data){

        for(const auto& stop : line.second){

            stop_in_vector = false;

            for(const auto& item : stops){
                if(item == stop.second.name){
                    stop_in_vector = true;
                }
            }
            if(!stop_in_vector){
                stops.push_back(stop.second.name);
            }
        }
    }

    std::cout << "All stops in alphabetical order:" << std::endl;
    std::sort(stops.begin(), stops.end());
    for(size_t i = 0; i < stops.size(); ++i){
        std::cout << stops.at(i) << std::endl;
    }
}


// Tulostaa yhden linjan pysäkit allekkain
void print_line(Tramway &rasse_data, const std::string &line_name)
{
    if (rasse_data.find(line_name) == rasse_data.end())
    {
        // Linjaa ei löydy mapista
        std::cout << "Error: Line could not be found." << std::endl;
    }
    else
    {
        // Linja löytyy
        std::cout << "Line " << line_name
                  << " goes through these stops in the order they are listed:"
                  << std::endl;

        for (auto &dist_stop_pair : rasse_data[line_name])
        {
            std::cout << "- " << dist_stop_pair.second.name << " : "
                      << dist_stop_pair.first << std::endl;
        }
    }
}

// Tarkistaa, löytyykö pysäkki Tramway-tietorakenteesta
bool stop_in_tramway(Tramway &rasse_data, const std::string &stop_name){

    bool stop_in_tramway = false;

    for(const auto& line : rasse_data){

        for(const auto& stop : line.second){

            if(stop.second.name == stop_name){

                stop_in_tramway = true;
            }
        }
    }
    return stop_in_tramway;
}


// Tulostaa allekkain kaikki linjat joille pysäkki kuuluu
void print_stop(Tramway &rasse_data, const std::string &stop_name){

    if(stop_in_tramway(rasse_data, stop_name)){

        std::cout << "Stop " << stop_name << " can be found on the following lines:" << std::endl;

        for(const auto& line : rasse_data){

            for(const auto& stop : line.second){

                if(stop.second.name == stop_name){

                    std::cout << " - " << line.first << std::endl;
                }
            }
        }
    }
    else{
        std::cout << "Error: Stop could not be found" << std::endl;
    }
}

// Käsittelee syötteen, joka sisältää ""-merkkejä"
std::vector<std::string> split_double_quotes(std::string &user_input){

    std::vector<std::string> parts = split(user_input, '"');
    std::vector<std::string> new_parts;
    std::string combined_string = "";
    bool double_quotes_found = false;

    for(std::string &p : parts){
        if(p.front() == '"'){
            double_quotes_found = true;

            // Poistetaan lainausmerkit edestä
            p.erase(p.begin());
            combined_string += p;
        }

        if(double_quotes_found && p.back() == '"'){

            // Poistetaan lainausmerkit perästä
            p.erase(p.end());
            combined_string += p;
            new_parts.push_back(combined_string);
            combined_string.erase();
            double_quotes_found = false;
        }

        else if(double_quotes_found){
            // Ollaan moniosaisen sanan välissä
            combined_string += p;
        }
        else{
            new_parts.push_back(p);
        }
    }
    new_parts.pop_back();
    return new_parts;
}


std::vector<std::string> ask_user_cmd()
{
    std::vector<std::string> input_fields;
    // Kysytään syöte
    std::cout << "tramway> ";
    std::string user_input = "";
    std::getline(std::cin, user_input);

    if(user_input.find('"') != std::string::npos){
        input_fields = split_double_quotes(user_input);
    }
    else{
        input_fields = split(user_input, ' ');
    }

    std::string command = input_fields.at(0);

    // Muutetaan komento suuriksi kirjaimiksi
    std::transform(command.begin(),
                   command.end(),
                   command.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    input_fields[0] = command;

    return input_fields;
}


bool interface(){

    // Luodaan tietorakenne linjoja/pysäkkejä varten
    Tramway rasse_data;

    // Kysytään käyttäjältä syötetiedosto
    std::cout << "Give a name for input file: " << std::endl;
    std::string filepath;
    std::getline(std::cin, filepath);
    std::vector<std::string> lines;

    // Luetaan tiedosto
    if(!read_input_file(filepath, lines)){
        return false;
    }

    // Käydään tiedosto läpi
    if(!parse_input_file(lines, rasse_data)){
        return false;
    }

    while(true){

        // Kysytään syöte
        std::vector<std::string> input = ask_user_cmd();

        // Poistetaan komennon lopusta mahdollinen turha välilyönti
        if(input[0].find(' ') != std::string::npos){
            input[0].pop_back();
        }

        // "QUIT"
        if(input[0] == "QUIT"){
            break;
        }

        // "LINES"
        else if(input[0] == "LINES"){
            print_lines(rasse_data);
        }

        // "LINE"
        else if (input[0] == "LINE")
        {
            if (input.size() == 2)
            {
                std::string line_name = input[1];
                print_line(rasse_data, line_name);
            }
            else
            {
                std::cout << "Error: Invalid input." << std::endl;
                continue;
            }
        }
        // "STOPS"
        else if(input[0] == "STOPS"){
            print_stops(rasse_data);
        }

        // "STOP"
        else if(input[0] == "STOP"){
            if(input.size() == 2){
                std::string stop_name = input[1];
                print_stop(rasse_data, stop_name);
            }
            else{
                std::cout << "Error: Invalid input." << std::endl;
                std::cout << "halo" << std::endl;
                continue;
            }
        }

        // Virheellinen komento
        else{
            std::cout << "Error: Invalid input." << std::endl;
            std::cout << "vituix" << std::endl;
        }

    }
    return true;
}


// Short and sweet main.
int main(){
    print_rasse();

    if (interface()){
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}
