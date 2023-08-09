#pragma once
#include "./game.h"
#include "./agents/weighted.h"
#include<thread>

struct WAgent : agents::Weighted
{
    float score;

    bool operator<(const WAgent &other) const
    {
        return score < other.score;
    }
    bool operator>(const WAgent &other) const
    {
        return score > other.score;
    }

    WAgent(const agents::Weighted& ag){
        this->search_depth = ag.search_depth;
        this->weights = ag.weights;
        score = 0;
    }
    WAgent() = default;
};
evaluators::MultistageEvaluationWeight generate_random_multistage_eval_weight(float max_val = 100)
{
    float op = randf(max_val);
    float mg = randf(max_val);
    float eg = randf(max_val);

    return {op, mg, eg};
}

evaluators::PerPiece generate_random_perpiece_eval_weight(float max = 100)
{
    return {
        randf(max),
        randf(max),
        randf(max),
        randf(max),
        randf(max),
        randf(max),
    };
}

WAgent generate_random_weighted_agent()
{
    WAgent wgt;
    wgt.weights.positioning = generate_random_multistage_eval_weight();
    wgt.weights.king_front_pawns = generate_random_multistage_eval_weight();
    wgt.weights.vulnerability = generate_random_multistage_eval_weight();
    wgt.weights.pawn_devel = generate_random_multistage_eval_weight();
    wgt.weights.mobility = generate_random_multistage_eval_weight();
    wgt.weights.material = generate_random_multistage_eval_weight();
    wgt.weights.check = generate_random_multistage_eval_weight();
    wgt.weights.center_control = generate_random_multistage_eval_weight();
    wgt.weights.protected_pieces = generate_random_multistage_eval_weight();

    wgt.weights.positions = generate_random_perpiece_eval_weight(100);
    wgt.weights.values = generate_random_perpiece_eval_weight(10000);
    return wgt;
}

///////////////////////
////// CONFIGURATION
///////////////////////

const float CROSSOVER_RATE = 0.2;
const float MUTATION_RATE = 0.5;
const float CLONE_RATE = 0.1;
const float REPEATED_MUTATION_CHANCE = 0.666;

const float MUTATION_FACTOR = 80;
const int GENERATION_COUNT = 1000;
const int GENERATION_SELECT = 20;
const int ITERATIONS = 100;
const auto processor_count = std::thread::hardware_concurrency();

#include <fstream>

enum Result
{
    WhiteWin,
    BlackWin,
    Draw,
};

int rand_weighted(std::vector<float> weights)
{
    float accum = 0;

    for (auto &w : weights)
    {
        w += accum;
        accum = w;
    }

    auto random_number = randf(accum);

    for (int i = 0; i < weights.size(); i++)
    {
        auto w = weights[i];
        if (w >= random_number)
        {
            return i;
        }
    }
    std::cout << "Random Element failed" << std::endl;
    exit(1);
}

std::map<std::string, Result> read_tuner_data()
{
    std::fstream data("../tuner_positions.txt");
    std::map<std::string, Result> ret;
    for (std::string line; getline(data, line);)
    {
        auto split = line.find("[");
        auto fen = line.substr(0, split - 1);
        auto num = line.substr(split);
        if (num == "[0.0]")
            ret[fen] = BlackWin;
        else if (num == "[0.5]")
            ret[fen] = Draw;
        else
            ret[fen] = WhiteWin;
    }
    return ret;
}

std::vector<WAgent> create_agents(int n)
{
    std::vector<WAgent> ret;
    for (int i = 0; i < n; i++)
    {
        ret.push_back(generate_random_weighted_agent());
    }
    return ret;
}
WAgent crossover_parents(std::vector<WAgent> parents)
{
    std::vector<float> weights;
    for (int i = 0; i < parents.size(); i++)
    {
        weights.push_back((parents.size() - i) * (parents.size() - i));
    }

    WAgent wgt;
    auto props = wgt.PARAMETERS();

    for (auto kvp : props)
    {
        auto chosen_parent = rand_weighted(weights);
        //        std::cout << "Choosing " << kvp.first << " from parent# " << chosen_parent << std::endl;
        if (auto msev = dynamic_cast<const evaluators::MultistageEvaluationWeight *>(kvp.second))
        {
            *(evaluators::MultistageEvaluationWeight *)msev = *(evaluators::MultistageEvaluationWeight *)parents[chosen_parent].PARAMETERS()[kvp.first];
        }
        else if (dynamic_cast<const evaluators::PerPiece *>(kvp.second))
        {
            *(evaluators::PerPiece *)(kvp.second) = *(evaluators::PerPiece *)parents[chosen_parent].PARAMETERS()[kvp.first];
        }
    }
    return wgt;
}
WAgent mutate_agent(WAgent p)
{
    WAgent copy = p;

    auto dict = copy.PARAMETERS();
    auto idx = rand() % dict.size();
    for (auto &p : dict)
    {
        if (idx == 0)
        {
            const_cast<chess::evaluators::EvalParameter *>(p.second)->mutate(MUTATION_FACTOR);
        }
        idx--;
    }

    return (randf(1) <= REPEATED_MUTATION_CHANCE) ? mutate_agent(copy) : copy;
}
std::vector<WAgent> generate_children(std::vector<WAgent> parents, int n)
{
    std::cout << "Parents: " << parents.size() << std::endl;
    std::vector<WAgent> ret;

    for (int i = 0; i < n; i++)
    {
        auto r = randf(1);
        if (r <= MUTATION_RATE)
        {
            std::vector<float> weights;
            std::transform(parents.begin(), parents.end(), std::back_inserter(weights), [](auto& ag){return ag.score;});
            auto chosen_idx = rand_weighted(weights);
            ret.push_back(mutate_agent(parents[chosen_idx]));
        }
        else if (r <= CROSSOVER_RATE + MUTATION_RATE)
        {
            ret.push_back(crossover_parents(parents));
        }
        else if(r <= CROSSOVER_RATE + MUTATION_RATE+CLONE_RATE)
        {
            //            std::cout << "Cloning best " << parents[0].encode() << std::endl;
            std::vector<float> weights;
            std::transform(parents.begin(), parents.end(), std::back_inserter(weights), [](auto& ag){return ag.score;});
            auto chosen_idx = rand_weighted(weights);
            ret.push_back(parents[chosen_idx]);
        }
        else{
            ret.push_back(generate_random_weighted_agent());
        }
    }
    return ret;
}

void populate_agent_score(WAgent &ag, std::map<std::string, Result> &dataset)
{
    int cnt = 0;
    for (auto g : dataset)
    {
        auto fen = g.first;
        auto res = g.second;
        auto game = Game::create(fen);
        if (game.fullmoves < 5)
        {
            cnt++;
            continue;
        }

        auto my_eval = ag.weightedsum(game, game.current_active_team());
        auto enemy_eval = ag.weightedsum(game, game.current_active_team() == Game::Team::White ? Game::Team::Black : Game::Team::White);

        if(game.current_active_team() == Game::Team::White){
            if(my_eval > enemy_eval && res == WhiteWin)cnt++;
            else if(my_eval < enemy_eval && res == BlackWin)cnt++;
        }
        else{
            if(my_eval > enemy_eval && res == BlackWin)cnt++;
            else if(my_eval < enemy_eval && res == WhiteWin)cnt++;
        }
        if (res == Draw)
        {
            auto diff = my_eval - enemy_eval;
            auto avg = (my_eval + enemy_eval) / 2;
            if (diff < 0)
                diff = -diff;
            // 10% threshold
            if (diff < (avg / 10))
                cnt++;
        }
    }
    ag.score = (float)cnt / (float)dataset.size();
}

void populate_agent_scores(std::vector<WAgent> &agents, std::map<std::string, Result> &dataset)
{
    for (auto &ag : agents)
    {
        populate_agent_score(ag, dataset);
    }
}

std::vector<WAgent> top_agents(std::vector<WAgent> agents, int n, std::map<std::string, Result> &dataset)
{

    auto worker_count = processor_count > 2 ? processor_count - 2 : 1;
    std::vector<std::thread> futures;
    std::vector<std::vector<WAgent>> workloads;
    workloads.reserve(worker_count); // Prevent U.B due to reallocation

    const auto worker_agent_count = agents.size() / worker_count;
    for (int i = 0; i < worker_count; i++)
    {
        auto start = worker_agent_count * i;
        auto end = worker_agent_count * (i + 1);
        auto workload = std::vector<WAgent>(agents.begin() + start, agents.begin() + end);
        workloads.push_back(workload);
        std::thread th(populate_agent_scores, std::ref(workloads[i]), std::ref(dataset));
        // auto res = std::async(populate_agent_scores, std::ref(workloads[i]), std::ref(dataset));
        futures.push_back(std::move(th));
    }
    for (auto &f : futures)
    {
        f.join();
    }

    std::vector<WAgent> best_agents;
    for (auto &wl : workloads)
    {
        for (auto &a : wl)
        {
            //        std::cout << "Score: " << agent_correct_guesses[i] << std::endl;
            auto score = a.score;
            if (best_agents.size() < n)
            {
                best_agents.push_back(a);
            }
            else
            {
                auto smallest = std::min_element(best_agents.begin(), best_agents.end());

                if (score > smallest->score)
                {
                    auto idx = smallest - best_agents.begin();
                    best_agents[idx] = a;
                }
            }
        }
    }

    std::sort(best_agents.begin(), best_agents.end(), [](auto &a1, auto &a2)
              { return a1 > a2; });
    return best_agents;
}

#include <chrono>

void trainify()
{

    std::cout << "Initial Agent Spec File? (leave blank for builtin default agent, or type random for a random start)" << std::endl;
    std::string specfile;
    
    std::getline(std::cin, specfile);

    auto data = read_tuner_data();
    srand(time(0));

    auto timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::ofstream logfile("Training Log " + std::to_string(timestamp));
    // Generate children seeded from the current weighted evaluator agent
    // we could generate random children, but that leads to a slower training
    // process, with some very questionable values, so it is generally preferred
    // to do it this way.

    logfile << "Chess Training Log @ " << timestamp << "\n";
    logfile << "Running with " << processor_count << " cores\n";
    logfile << "Mutation Rate: " << MUTATION_RATE << "\n";
    logfile << "Mutation Factor: " << MUTATION_FACTOR << "\n";
    logfile << "Crossover Rate: " << CROSSOVER_RATE << "\n";
    logfile << "Agents per Generation: " << GENERATION_COUNT << "\n";
    logfile << "Champions per Generation: " << GENERATION_SELECT << "\n";


    std::vector<WAgent> agents;

    if(specfile == "random"){
        for(int i = 0; i < GENERATION_COUNT; i++){
            agents.push_back(generate_random_weighted_agent());
        }
    }
    else if(specfile.empty()){
            WAgent default_agent = agents::Weighted();
            populate_agent_score(default_agent, data);
            logfile << "Initial agent score: " << default_agent.score << "\n";
            logfile << "Initial Configuration:\n"
            << default_agent.encode() << "\n";
            agents = generate_children({default_agent}, GENERATION_COUNT);
    }
    else{
            WAgent default_agent = agents::Weighted::from_file(specfile);
            populate_agent_score(default_agent, data);
            logfile << "Initial agent score: " << default_agent.score << "\n";
            logfile << "Initial Configuration:\n"
            << default_agent.encode() << "\n";
            agents = generate_children({default_agent}, GENERATION_COUNT);
    }



    for (uint32_t i = 1; i <= ITERATIONS; i++)
    {
        logfile << "---\n\n";
        std::cout << "==============================================" << std::endl;
        std::cout << "========        GENERATION " << i << "         =========" << std::endl;
        std::cout << "==============================================" << std::endl;
        auto best_agents = top_agents(agents, GENERATION_SELECT, data);
        std::cout << "Top performer: " << best_agents[0].score << std::endl;
        logfile << "Generation " << i << "\n";
        logfile << "Best Score: " << best_agents[0].score << "\n";

        logfile << "<< Agents >>\n";
        int idx = 1;
        for (auto &ag : best_agents)
        {
            logfile << "Agent #" << idx << "\n";
            logfile << "Score: " << ag.score << "\n";
            logfile << ag.encode() << "\n";
            idx++;
        }
        // We Write information relating to the current generation to the log file
        // The log file can then be parsed by a simple python script to run statistics

        auto next_generation = generate_children(best_agents, GENERATION_COUNT);

        agents = next_generation;
    }
}