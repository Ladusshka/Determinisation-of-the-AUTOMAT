#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using State = unsigned;
using Symbol = uint8_t;
using Word = std::vector<Symbol>;

struct MISNFA {
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    std::set<State> initialStates;
    std::set<State> finalStates;
};

struct DFA {
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, State> transitions;
    State initialState;
    std::set<State> finalStates;

    bool operator==(const DFA& x) const = default; // requires c++20
};
#endif




MISNFA EpsilonElimination(const MISNFA& nfa){

    std::map<State, std::set<State>> EpsilonClosures; // to which states is possible to get by epsilon movement


    for(auto & state : nfa.states){ // for each state bfs

        std::set<State> visited;
        std::queue<State> queue;
        visited.insert(state);
        queue.push(state);

        while(!queue.empty()){
            auto current = queue.front();
            queue.pop();

            auto it = nfa.transitions.find({current, 'e'});
            if (it != nfa.transitions.end()){
                auto ways = nfa.transitions.find({current,'e'})->second; // set of states by epsilon
                for(auto & state_to_go : ways){
                    if(!visited.contains(state_to_go)){
                        visited.insert(state_to_go);
                        queue.push(state_to_go);
                    }
                }
            }


        }

        EpsilonClosures[state] = std::move(visited);
    }

    MISNFA newNfa = nfa;  // Создаем копию исходного автомата
    newNfa.transitions.clear();
    newNfa.alphabet.erase('e');
    for (const auto &state_pair : EpsilonClosures) {
        State state = state_pair.first; // Текущее состояние
        const std::set<State> &closure = state_pair.second; // Closure для этого состояния

        for (auto &state_to_add : closure) {
            for (auto &symbol : nfa.alphabet) {
                if (symbol == 'e') continue; // Пропускаем ε

                auto transition_original_nfa = nfa.transitions.find({state_to_add, symbol});
                if (transition_original_nfa != nfa.transitions.end()) {
                    for (auto &nfa_ways : transition_original_nfa->second) {

                        newNfa.transitions[{state, symbol}].insert(nfa_ways);
                    }
                }
            }
        }
    }



    for (const auto &state_pair : EpsilonClosures) {
        State state = state_pair.first;
        const std::set<State> &closure = state_pair.second;

        for (auto closureState : closure) {
            if (nfa.finalStates.find(closureState) != nfa.finalStates.end()) {
                newNfa.finalStates.insert(state);
                break;
            }
        }
    }

    return newNfa;



}
void makeTotalDFA(DFA& m_DFA) {
    if (m_DFA.states.empty()) {
        return;
    }

    State deadState = *std::max_element(m_DFA.states.begin(), m_DFA.states.end()) + 1;

    bool needDeadState = false;

    for (State s : m_DFA.states) {
        for (Symbol c : m_DFA.alphabet) {
            if (m_DFA.transitions.find({s, c}) == m_DFA.transitions.end()) {
                needDeadState = true;
                break;
            }
        }
        if (needDeadState) {
            break;
        }
    }

    if (!needDeadState) {
        return;
    }

    m_DFA.states.insert(deadState);
    for (Symbol c : m_DFA.alphabet) {
        m_DFA.transitions[{deadState, c}] = deadState;
    }

    for (State s : m_DFA.states) {
        for (Symbol c : m_DFA.alphabet) {
            if (m_DFA.transitions.find({s, c}) == m_DFA.transitions.end()) {
                m_DFA.transitions[{s, c}] = deadState;
            }
        }
    }
}




DFA deleting_unreachable(DFA & start_dfa){

    std::set<State> visited;
    std::queue<State> queue;

    visited.insert(start_dfa.initialState);
    queue.push(start_dfa.initialState);

    while(!queue.empty()){

        auto current = queue.front();
        queue.pop();

        for(auto symbol : start_dfa.alphabet){
            auto res = start_dfa.transitions.find({current, symbol});
            if(res != start_dfa.transitions.end()){
                if(!visited.contains(res->second)){
                    visited.insert(res->second);
                    queue.push(res->second);
                }
            }
        }
    }

    DFA newDFA ;
    newDFA.states = visited;
    newDFA.alphabet = start_dfa.alphabet;
    newDFA.initialState = start_dfa.initialState;

    for(auto state : newDFA.states){
        for(auto symbol : newDFA.alphabet){
            auto res = start_dfa.transitions.find({state, symbol});
            if( res != start_dfa.transitions.end()){
                newDFA.transitions[{state, symbol}] = res->second;
            }
        }
    }

    for(auto state : newDFA.states){
       if(start_dfa.finalStates.contains(state)){
           newDFA.finalStates.insert(state);
       }
    }


    return newDFA;

}


DFA deleting_useless(DFA && dfa) {
    std::set<State> usefulStates;
    std::queue<State> queue;

    // Инициализируем очередь конечными состояниями, так как ищем состояния, ведущие к ним
    for (auto final : dfa.finalStates) {
        usefulStates.insert(final);
        queue.push(final);
    }

    // BFS/DFS для нахождения всех состояний, которые могут достичь конечного состояния
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        // Ищем все состояния, из которых есть переход в текущее
        for (auto symbol : dfa.alphabet) {
            for (const auto& [transition, targetState] : dfa.transitions) {
                if (transition.second == symbol && targetState == current) {
                    State fromState = transition.first;
                    if (usefulStates.find(fromState) == usefulStates.end()) {
                        usefulStates.insert(fromState);
                        queue.push(fromState);
                    }
                }
            }
        }
    }

    // Создаем новый DFA только с полезными состояниями и их переходами
    DFA newDFA;
    newDFA.states = usefulStates;
    newDFA.alphabet = dfa.alphabet;
    newDFA.initialState = dfa.initialState;

    // Переносим только переходы, которые связаны с полезными состояниями
    for (auto state : newDFA.states) {
        for (auto symbol : newDFA.alphabet) {
            auto res = dfa.transitions.find({state, symbol});
            if (res != dfa.transitions.end() && usefulStates.find(res->second) != usefulStates.end()) {
                newDFA.transitions[{state, symbol}] = res->second;
            }
        }
    }

    // Переносим конечные состояния, которые являются полезными
    newDFA.finalStates = dfa.finalStates;

    return newDFA;
}





DFA determinize(const MISNFA& nfa) {
    DFA dfa;
    std::map<std::set<State>, State> stateMapping;
    std::queue<std::set<State>> queue;
    State nextDfaState = 0;

    // Начальное состояние DFA — это множество начальных состояний NFA
    std::set<State> startStateSet = nfa.initialStates;
    stateMapping[startStateSet] = nextDfaState;
    dfa.initialState = nextDfaState;
    dfa.states.insert(nextDfaState);
    queue.push(startStateSet);
    nextDfaState++;

    // Обрабатываем состояния очереди
    while (!queue.empty()) {
        std::set<State> currentSet = queue.front();
        queue.pop();
        State currentDfaState = stateMapping[currentSet];

        // Проверяем каждый символ алфавита
        for (Symbol symbol : nfa.alphabet) {
            std::set<State> nextSet;

            // Собираем все состояния, достижимые из текущего множества для символа
            for (State state : currentSet) {
                auto transition = nfa.transitions.find({state, symbol});
                if (transition != nfa.transitions.end()) {
                    nextSet.insert(transition->second.begin(), transition->second.end());
                }
            }

            // Если есть переходы для символа
            if (!nextSet.empty()) {
                // Если новое множество состояний не было ранее добавлено
                if (stateMapping.find(nextSet) == stateMapping.end()) {
                    stateMapping[nextSet] = nextDfaState;
                    dfa.states.insert(nextDfaState);
                    queue.push(nextSet);
                    nextDfaState++;
                }

                // Добавляем переход в DFA
                dfa.transitions[{currentDfaState, symbol}] = stateMapping[nextSet];
            }
        }
    }

    // Определяем конечные состояния DFA
    for (const auto& [stateSet, dfaState] : stateMapping) {
        for (State state : stateSet) {
            if (nfa.finalStates.count(state)) {
                dfa.finalStates.insert(dfaState);
                break;
            }
        }
    }

    dfa.alphabet = nfa.alphabet;
    return dfa;
}


DFA complement(const MISNFA& nfa){
   // DFA dfa  = determinize(EpsilonElimination(nfa));
    DFA dfa  = determinize(nfa);
    makeTotalDFA(dfa);
    std::set<State> newFinalStates;
    for (State state : dfa.states) {
        if (dfa.finalStates.find(state) == dfa.finalStates.end()) {
            newFinalStates.insert(state);
        }
    }
   dfa.finalStates = newFinalStates;

    DFA dfa_without_unreachable_useless = deleting_useless(deleting_unreachable(dfa));

    if (dfa_without_unreachable_useless.states.empty()) {
        dfa_without_unreachable_useless.initialState = 0;
        dfa_without_unreachable_useless.states.insert(0);
        dfa_without_unreachable_useless.alphabet = nfa.alphabet;
        dfa_without_unreachable_useless.transitions.clear();
        for (Symbol c : dfa_without_unreachable_useless.alphabet) {
            dfa_without_unreachable_useless.transitions[{0, c}] = 0;
        }
        dfa_without_unreachable_useless.finalStates.clear();
    }

    return dfa_without_unreachable_useless;



}
bool run(const DFA& m_DFA, const Word& m_Word) {
    State m_Current = m_DFA.initialState;
    for (const Symbol &m_Symbol : m_Word) {
        auto it = m_DFA.transitions.find({m_Current, m_Symbol});
        if (it == m_DFA.transitions.end()) {
            return false;
        }
        m_Current = it->second;
    }
    return m_DFA.finalStates.find(m_Current) != m_DFA.finalStates.end();
}

#ifndef __PROGTEST__
int main()
{
    MISNFA in0 = {
            {0, 1, 2},
            {'e', 'l'},
            {
             {{0, 'e'}, {1}},
                    {{0, 'l'}, {1}},
                   {{1, 'e'}, {2}},
                    {{2, 'e'}, {0}},
                    {{2, 'l'}, {2}},
            },
            {0},
            {1, 2},
    };
    auto out0 = complement(in0);
   // assert(run(out0, {}) == true);
    assert(run(out0, {'e', 'l'}) == true);
    assert(run(out0, {'l'}) == false);
    assert(run(out0, {'l', 'e', 'l', 'e'}) == true);
    MISNFA in1 = {
            {0, 1, 2, 3},
            {'g', 'l'},
            {
             {{0, 'g'}, {1}},
                    {{0, 'l'}, {2}},
                   {{1, 'g'}, {3}},
                      {{1, 'l'}, {3}},
                    {{2, 'g'}, {1}},
                    {{2, 'l'}, {0}},
                    {{3, 'l'}, {1}},
            },
            {0},
            {0, 2, 3},
    };
    auto out1 = complement(in1);
    assert(run(out1, {}) == false);
    assert(run(out1, {'g'}) == true);
    assert(run(out1, {'g', 'l'}) == false);
    assert(run(out1, {'g', 'l', 'l'}) == true);
    MISNFA in2 = {
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
            {'a', 'b'},
            {
             {{0, 'a'}, {1}},
                    {{0, 'b'}, {2}},
                   {{1, 'a'}, {3}},
                      {{1, 'b'}, {4}},
                         {{2, 'a'}, {5}},
                            {{2, 'b'}, {6}},
                               {{3, 'a'}, {7}},
                                  {{3, 'b'}, {8}},
                                     {{4, 'a'}, {9}},
                                        {{5, 'a'}, {5}},
                                           {{5, 'b'}, {10}},
                                               {{6, 'a'}, {8}},
                    {{6, 'b'}, {8}},
                    {{7, 'a'}, {11}},
                    {{8, 'a'}, {3}},
                    {{8, 'b'}, {9}},
                    {{9, 'a'}, {5}},
                    {{9, 'b'}, {5}},
                    {{10, 'a'}, {1}},
                    {{10, 'b'}, {2}},
                    {{11, 'a'}, {5}},
                    {{11, 'b'}, {5}},
            },
            {0},
            {5, 6},
    };
    auto out2 = complement(in2);
    assert(run(out2, {}) == true);
    assert(run(out2, {'a'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'a', 'a', 'b'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'c', 'a', 'a'}) == false);
    MISNFA in3 = {
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
            {'e', 'j', 'k'},
            {
             {{0, 'e'}, {1}},
                    {{0, 'j'}, {2}},
                    {{0, 'k'}, {3}},
                      {{1, 'e'}, {2}},
                         {{1, 'j'}, {4}},
                            {{1, 'k'}, {5}},
                               {{2, 'e'}, {6}},
                                  {{2, 'j'}, {0}},
                                     {{2, 'k'}, {4}},
                                        {{3, 'e'}, {7}},
                    {{3, 'j'}, {2}},
                    {{3, 'k'}, {1}},
                    {{4, 'e'}, {4}},
                    {{4, 'j'}, {8}},
                    {{4, 'k'}, {7}},
                    {{5, 'e'}, {4}},
                    {{5, 'j'}, {0}},
                    {{5, 'k'}, {4}},
                    {{6, 'e'}, {7}},
                    {{6, 'j'}, {8}},
                    {{6, 'k'}, {4}},
                    {{7, 'e'}, {3}},
                    {{7, 'j'}, {1}},
                    {{7, 'k'}, {8}},
                    {{8, 'e'}, {2}},
                    {{8, 'j'}, {4}},
                    {{8, 'k'}, {9}},
                    {{9, 'e'}, {4}},
                    {{9, 'j'}, {0}},
                    {{9, 'k'}, {4}},
            },
            {0},
            {1, 6, 8},
    };
    auto out3 = complement(in3);
    assert(run(out3, {}) == true);
    assert(run(out3, {'b', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'j'}) == true);
    assert(run(out3, {'e', 'k', 'j', 'e', 'j', 'j', 'j', 'e', 'k'}) == true);
    MISNFA in4 = {
            {0, 1, 2, 3, 4, 5},
            {'e', 'n', 'r'},
            {
             {{0, 'e'}, {1}},
                    {{0, 'n'}, {1}},
                    {{0, 'r'}, {2}},
                      {{1, 'e'}, {2}},
                         {{1, 'n'}, {3}},
                            {{1, 'r'}, {3}},
                    {{2, 'e'}, {3}},
                    {{2, 'n'}, {3}},
                    {{2, 'r'}, {1}},
                    {{3, 'e'}, {1}},
                    {{3, 'n'}, {1}},
                    {{3, 'r'}, {2}},
                    {{4, 'r'}, {5}},
            },
            {0},
            {4, 5},
    };
    auto out4 = complement(in4);
    assert(run(out4, {}) == true);
    assert(run(out4, {'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r'}) == true);
    assert(run(out4, {'n', 'e', 'r', 'n', 'r', 'r', 'r', 'n', 'e', 'n', 'n', 'm', '\x0c', '\x20'}) == false);
    assert(run(out4, {'r', 'r', 'r', 'e', 'n'}) == true);
    MISNFA in5 = {
            {0, 1, 2, 3, 4, 5, 6},
            {'l', 'q', 't'},
            {
             {{0, 'l'}, {2, 4, 5}},
                    {{0, 'q'}, {2}},
                    {{0, 't'}, {1}},
                      {{1, 'l'}, {0, 2}},
                         {{1, 'q'}, {1, 4}},
                            {{1, 't'}, {0, 2}},
                               {{2, 'l'}, {5}},
                    {{2, 'q'}, {0, 4}},
                    {{2, 't'}, {0}},
                    {{3, 'l'}, {3, 4}},
                    {{3, 'q'}, {0}},
                    {{3, 't'}, {0, 2}},
                    {{4, 't'}, {4}},
                    {{5, 'l'}, {0, 2}},
                    {{5, 'q'}, {4, 5}},
                    {{5, 't'}, {0, 2}},
                    {{6, 'l'}, {4, 6}},
                    {{6, 'q'}, {0}},
                    {{6, 't'}, {0, 2}},
            },
            {2, 4},
            {0, 1, 3, 5, 6},
    };
    auto out5 = complement(in5);
    assert(run(out5, {}) == true);
    assert(run(out5, {'q', 'q'}) == true);
    assert(run(out5, {'q', 'q', 'l'}) == false);
    assert(run(out5, {'q', 'q', 'q'}) == false);
}
#endif
