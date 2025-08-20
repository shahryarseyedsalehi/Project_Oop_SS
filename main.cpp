#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <map>
#include <sstream>
#include <cmath>
#include <windows.h>
#include <SDL2/SDL_image.h>
#include <SDL_image.h>
#include <SDL2/SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#include <algorithm>
#include <windows.h>
using namespace std;


class Node {
private:
    string name;
    int number;
    double voltage;


public:
    int x, y;
    string getName()
    {
        return name;
    }
    void setName(const string& newName) {
        name = newName;
    }
    double getVoltage()
    {
        return voltage;
    }

    Node(string input, int n, int grid_x = -1, int grid_y = -1)
            : name(input), number(n), voltage(0.0), x(grid_x), y(grid_y) {}

    void setVoltage (double v)
    {
        voltage = v;
    }
};


class Component {
protected:
    string name;
    string type;
    Node* node1;
    Node* node2;
    double value;

public:
    Component(string input, Node* n1, Node* n2, string t, double v1) : name(input), node1(n1), node2(n2), type(t),value(v1) {}
    virtual double getVoltage()  = 0;
    virtual double getCurrent()  = 0;
    virtual void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) {}

    virtual void setValue(double v)  {}
    virtual void update_state(double current = 0) {

    }
    virtual double getRes() const { return 0.0; }
    virtual double getInd() const { return 0.0; }
    virtual double getCap() const { return 0.0; }
    virtual double getPrevVoltage() const { return 0.0; }

    string getName()
    {
        return name;
    }
    string getType()
    {
        return type;
    }
    double getvalue()
    {
        return value;
    }
    Node* getNode1()
    {
        return node1;

    }

    Node* getNode2()  {

        return node2;
    }
    void setName(const string& newName) {
        name = newName;
    }


};



vector< string> Error;
vector<Node*> nodes;
vector<Component*> Components;
Node* GN = nullptr;

int Node_count = 0;


Node* findnode(string name)
{
    for (Node* nod : nodes)
    {
        if (nod->getName() == name)
        {
            return nod;
        }
    }
    return nullptr;
}

Component* findComponent( string name)
{
    for (Component* comp : Components)
    {
        if (comp->getName() == name)
        {
            return comp;
        }

    }
    return nullptr;
}



class Resistor : public Component {
private:
    double resistance;

public:
    Resistor(string input, Node* n1, Node* n2, double r): Component(input, n1, n2, "Resistor",r), resistance(r) {}

    double getVoltage()  override
    {
        return abs(node1->getVoltage() - node2->getVoltage());
    }
    double getRes() const override { return resistance; }
    double getCurrent()  override
    {
        return (node1->getVoltage() - node2->getVoltage()) / resistance;
    }
    void setValue(double r) override {
        if (r > 0) {
            resistance = r;
            value = r; // Also update the base class value
        }
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        double g = 1.0 / resistance;
        if (node1 != GN) A[node_map.at(node1->getName())][node_map.at(node1->getName())] += g;
        if (node2 != GN) A[node_map.at(node2->getName())][node_map.at(node2->getName())] += g;
        if (node1 != GN && node2 != GN) {
            A[node_map.at(node1->getName())][node_map.at(node2->getName())] -= g;
            A[node_map.at(node2->getName())][node_map.at(node1->getName())] -= g;
        }
    }


};
class Capacitor : public Component {
private:
    double capacitance;
    double prev_voltage;

public:
    Capacitor(string input, Node* n1, Node* n2, double c): Component(input, n1, n2, "Capacitor",c), capacitance(c), prev_voltage(0.0) {}

    double getVoltage()  override
    {
        return abs(node1->getVoltage() - node2->getVoltage());
    }

    double getCurrent()  override
    {
        return 0.0;
    }
    void setValue(double c) override {
        if (c > 0) {
            capacitance = c;
            value = c;
        }
    }

    double getCap() const override { return capacitance; }
    double getPrevVoltage() const override { return prev_voltage; }

    void update_state(double current = 0) override {
        prev_voltage = node1->getVoltage() - node2->getVoltage();
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        double g;
        if (time_step == 0) {
            g = 1e-12;
        } else {
            g = capacitance / time_step;
        }
        double isource = g * prev_voltage;

        if (node1 != GN) {
            A[node_map.at(node1->getName())][node_map.at(node1->getName())] += g;
            b[node_map.at(node1->getName())] += isource;
        }
        if (node2 != GN) {
            A[node_map.at(node2->getName())][node_map.at(node2->getName())] += g;
            b[node_map.at(node2->getName())] -= isource;
        }
        if (node1 != GN && node2 != GN) {
            A[node_map.at(node1->getName())][node_map.at(node2->getName())] -= g;
            A[node_map.at(node2->getName())][node_map.at(node1->getName())] -= g;
        }
    }
};

class Inductor : public Component {
private:
    double inductance;
    double prev_current;
public:
    Inductor(string input, Node* n1, Node* n2, double l): Component(input, n1, n2, "Inductor",l), inductance(l), prev_current(0.0) {}

    double getVoltage()  override
    {
        return abs(node1->getVoltage() - node2->getVoltage());
    }

    double getInd() const override { return inductance; }
    double getCurrent()  override
    {
        return prev_current;
    }

    void update_state(double current) override {
        prev_current = current;
    }
    void setValue(double l) override {
        if (l > 0) {
            inductance = l;
            value = l;
        }
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        double g;
        if (time_step == 0) {
            g = 1e9;
        } else {
            g = time_step / inductance;
        }

        double isource = prev_current;

        if (node1 != GN) {
            A[node_map.at(node1->getName())][node_map.at(node1->getName())] += g;
            b[node_map.at(node1->getName())] -= isource;
        }
        if (node2 != GN) {
            A[node_map.at(node2->getName())][node_map.at(node2->getName())] += g;
            b[node_map.at(node2->getName())] += isource;
        }
        if (node1 != GN && node2 != GN) {
            A[node_map.at(node1->getName())][node_map.at(node2->getName())] -= g;
            A[node_map.at(node2->getName())][node_map.at(node1->getName())] -= g;
        }
    }
};
class Diode : public Component {
private:

    string model_name;
    double Diode_voltage;
public:
    Diode(string input, Node* n1, Node* n2, string model, double zener_v = 0.0) : Component(input, n1, n2, "Diode",0.0), model_name(model), Diode_voltage(zener_v) {}

    double getVoltage() override
    {
        if (model_name == "D") {
            return 0.0;
        }
        else if (model_name == "Z") {
            double v1 = node1->getVoltage();
            double v2 = node2->getVoltage();
            double diff_voltage = v1 - v2;

            if (diff_voltage >= 0.7) {
                return 0.7;
            }
            else if (diff_voltage <= -Diode_voltage && Diode_voltage > 0) {
                return Diode_voltage;
            }

            else {
                return abs(diff_voltage);
            }
        }
        return 0.0;
    }

    double getCurrent() override
    {
        return 0.0;
    }

    string getModelName()
    {
        return model_name;
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        double Vd = node1->getVoltage() - node2->getVoltage();
        double Is = 1e-14;
        double Vt = 0.02585;

        double Id = Is * (exp(Vd / Vt) - 1.0);
        double Gd = (Is / Vt) * exp(Vd / Vt);


        int n1_idx;
        if (node1 == GN)
        {
            n1_idx = -1;
        }
        else {
            n1_idx = node_map.at(node1->getName());
        }


        int n2_idx;
        if (node2 == GN)
        {
            n2_idx = -1;
        }
        else {
            n2_idx = node_map.at(node2->getName());
        }

        if (n1_idx != -1) {
            A[n1_idx][n1_idx] += Gd;
            b[n1_idx] -= (Id - Gd * Vd);
        }
        if (n2_idx != -1) {
            A[n2_idx][n2_idx] += Gd;
            b[n2_idx] += (Id - Gd * Vd);
        }
        if (n1_idx != -1 && n2_idx != -1) {
            A[n1_idx][n2_idx] -= Gd;
            A[n2_idx][n1_idx] -= Gd;
        }
    }
};
class DcVoltageSource : public Component {
private:
    double value;

public:
    DcVoltageSource(string input, Node* n1, Node* n2, double val)
            : Component(input, n1, n2, "DcVoltageSource",val), value(val) {}

    double getVoltage() override {
        return value;
    }

    void setValue(double v) override {
        value = v;
    }

    double getCurrent() override {
        return 0.0;
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        int v_idx = node_map.at(this->getName());

        if (node1 != GN)
        {
            A[v_idx][node_map.at(node1->getName())] = 1.0;
            A[node_map.at(node1->getName())][v_idx] = 1.0;
        }
        if (node2 != GN) {
            A[v_idx][node_map.at(node2->getName())] = -1.0;
            A[node_map.at(node2->getName())][v_idx] = -1.0;
        }
        b[v_idx] = this->getVoltage();
    }
};

class SinVoltageSource : public Component {
private:
    double offset, amp, frequency;
public:
    SinVoltageSource(string input, Node* n1, Node* n2, double o, double a, double f) : Component(input, n1, n2, "SinVoltageSource",a), offset(o), amp(a), frequency(f) {}
    double getVoltageAtTime(double time)
    {
        return offset + amp * sin(2 * 3.1415926535 * frequency * time);
    }
    double getVoltage() override
    {
        return getVoltageAtTime(0);
    }
    double getCurrent() override {
        return 0.0;
    }
    void setSinValues(double o, double a, double f) {
        if (f > 0) {
            offset = o;
            amp = a;
            frequency = f;
        }
    }
    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {
        int v_idx = node_map.at(this->getName());

        if (node1 != GN)
        {
            A[v_idx][node_map.at(node1->getName())] = 1.0;
            A[node_map.at(node1->getName())][v_idx] = 1.0;
        }
        if (node2 != GN)
        {
            A[v_idx][node_map.at(node2->getName())] = -1.0;
            A[node_map.at(node2->getName())][v_idx] = -1.0;
        }
        b[v_idx] = this->getVoltageAtTime(time);
    }
};


class CurrentSource : public Component {
private:
    double value;
public:
    CurrentSource(string input, Node* n1, Node* n2, double val): Component(input, n1, n2, "CurrentSource",val), value(val) {}

    double getVoltage() override {
        return 0.0;
    }

    double getCurrent() override {

        return value;
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double time_step) override {

        int n1_idx;
        if (node1 == GN)
        {
            n1_idx = -1;
        }
        else
        {
            n1_idx = node_map.at(node1->getName());
        }


        int n2_idx;
        if (node2 == GN)
        {
            n2_idx = -1;
        }
        else
        {
            n2_idx = node_map.at(node2->getName());
        }

        if (n1_idx != -1) {
            b[n1_idx] -= value;
        }
        if (n2_idx != -1) {
            b[n2_idx] += value;
        }
    }
};
class WaveformSource : public Component {
private:
    vector<double> voltage_values;
    double time_step;

public:
    WaveformSource(string input, Node* n1, Node* n2, const vector<double>& values, double step)
            : Component(input, n1, n2, "WaveformSource", 0.0), voltage_values(values), time_step(step) {}

    double getVoltageAtTime(double time) {
        if (time_step <= 0 || voltage_values.empty()) {
            return 0.0;
        }
        int index = static_cast<int>(floor(time / this->time_step));

        if (index >= 0 && index < voltage_values.size()) {
            return voltage_values[index];
        }

        if (!voltage_values.empty()) {
            return voltage_values.back();
        }
        return 0.0;
    }

    double getVoltage() override {

        return getVoltageAtTime(0);
    }

    double getCurrent() override {

        return 0.0;
    }

    void stamp(vector<vector<double>>& A, vector<double>& b, const map<string, int>& node_map, double time, double sim_time_step) override {
        int v_idx = node_map.at(this->getName());

        if (node1 != GN) {
            A[v_idx][node_map.at(node1->getName())] = 1.0;
            A[node_map.at(node1->getName())][v_idx] = 1.0;
        }
        if (node2 != GN) {
            A[v_idx][node_map.at(node2->getName())] = -1.0;
            A[node_map.at(node2->getName())][v_idx] = -1.0;
        }

        b[v_idx] = this->getVoltageAtTime(time);
    }
};

class Circuit {
public:

    vector<double> solveLU(vector<vector<double>>& A, vector<double>& b)
    {
        int n = b.size();
        if (n == 0) return {};

        vector<vector<double>> L(n, vector<double>(n, 0));
        vector<vector<double>> U(n, vector<double>(n, 0));

        for (int i = 0; i < n; i++) {
            for (int k = i; k < n; k++) {
                double sum = 0;
                for (int j = 0; j < i; j++)
                    sum += (L[i][j] * U[j][k]);
                U[i][k] = A[i][k] - sum;
            }
            for (int k = i; k < n; k++)
            {
                if (i == k)
                    L[i][i] = 1;
                else
                {
                    double sum = 0;
                    for (int j = 0; j < i; j++)
                        sum += (L[k][j] * U[j][i]);
                    if (U[i][i] == 0)
                        return {};
                    L[k][i] = (A[k][i] - sum) / U[i][i];
                }
            }
        }

        vector<double> y(n, 0);
        for (int i = 0; i < n; i++)
        {
            double sum = 0;
            for (int j = 0; j < i; j++)
            {
                sum += L[i][j] * y[j];
            }
            if (L[i][i] == 0)
                return {};

            y[i] = (b[i] - sum) / L[i][i];
        }

        vector<double> x(n, 0);
        for (int i = n - 1; i >= 0; i--)
        {
            double sum = 0;

            for (int j = i + 1; j < n; j++)
            {
                sum += U[i][j] * x[j];
            }

            if (U[i][i] == 0)
                return {};

            x[i] = (y[i] - sum) / U[i][i];
        }
        return x;
    }

    double get_printable_value(const string& var, const map<string, int>& node_map, const vector<double>& x, double time_step = 0) {
        char type = var.front();
        string name = var.substr(2, var.length() - 3);

        if (type == 'V')
        {
            Node* n = findnode(name);
            if (n == GN)
                return 0.0;
            if (n && node_map.count(name))
                return x.at(node_map.at(name));
        }
        else if (type == 'I')
        {
            Component* c = findComponent(name);
            if (c) {
                if (c->getType() == "DcVoltageSource" || c->getType() == "SinVoltageSource")
                {
                    if (node_map.count(name)) return x.at(node_map.at(name));
                }
                else if (c->getType() == "Resistor") {

                    double v1;
                    if (c->getNode1() == GN)
                    {
                        v1 = 0;
                    }
                    else {
                        v1 = x.at(node_map.at(c->getNode1()->getName()));
                    }


                    double v2;
                    if (c->getNode2() == GN) {
                        v2 = 0;
                    } else {
                        v2 = x.at(node_map.at(c->getNode2()->getName()));
                    }
                    return (v1 - v2) / c->getRes();
                }
                else if (c->getType() == "Inductor")
                {
                    double v1;
                    if (c->getNode1() == GN) {
                        v1 = 0;
                    } else {
                        v1 = x.at(node_map.at(c->getNode1()->getName()));
                    }

                    double v2;
                    if (c->getNode2() == GN) {
                        v2 = 0;
                    } else {
                        v2 = x.at(node_map.at(c->getNode2()->getName()));
                    }
                    return (v1 - v2) / (c->getInd() / time_step) + c->getCurrent();
                }
                else if (c->getType() == "Capacitor")
                {
                    if (time_step == 0)
                        return 0.0;
                    double v1;
                    if (c->getNode1() == GN)
                    {
                        v1 = 0;
                    }
                    else {
                        v1 = x.at(node_map.at(c->getNode1()->getName()));
                    }

                    double v2;
                    if (c->getNode2() == GN)
                    {
                        v2 = 0;
                    }
                    else {
                        v2 = x.at(node_map.at(c->getNode2()->getName()));
                    }
                    return c->getCap() * ((v1 - v2) - c->getPrevVoltage()) / time_step;
                }
            }
        }
        return NAN;
    }

    vector<double> solve_system(int matrix_size, const map<string, int>& node_map, double time, double time_step) {
        bool has_nonlinear = false;
        for (auto comp : Components) {
            if (comp->getType() == "Diode") {
                has_nonlinear = true;
                break;
            }
        }

        if (!has_nonlinear) {
            vector<vector<double>> A(matrix_size, vector<double>(matrix_size, 0.0));
            vector<double> b(matrix_size, 0.0);
            for (auto comp : Components) {
                comp->stamp(A, b, node_map, time, time_step);
            }
            return solveLU(A, b);
        }

        vector<double> x_guess(matrix_size, 0.0);
        int max_iterations = 100;
        double tolerance = 1e-6;

        for (int i = 0; i < max_iterations; ++i)
        {
            for (auto const& pair : node_map)
            {
                if (findnode(pair.first))
                {
                    findnode(pair.first)->setVoltage(x_guess[pair.second]);
                }
            }

            vector<vector<double>> J(matrix_size, vector<double>(matrix_size, 0.0));
            vector<double> F(matrix_size, 0.0);

            for (auto comp : Components) {
                comp->stamp(J, F, node_map, time, time_step);
            }

            vector<double> delta_x = solveLU(J, F);
            if (delta_x.empty()) return {};

            double norm = 0.0;
            for(size_t j=0; j<matrix_size; ++j) {
                x_guess[j] += delta_x[j];
                norm += delta_x[j] * delta_x[j];
            }

            if (sqrt(norm) < tolerance) {
                return x_guess;
            }
        }
        return {};
    }

    void performTransientAnalysis(double t_step, double t_stop, double t_start, const vector<string>& vars_to_print)
    {
        if (GN == nullptr)
        {
            cout << "Error: GND node not set." << endl;
            return;
        }

        int voltage_source_count = 0;
        for (auto comp : Components)
        {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource") {
                voltage_source_count++;
            }
        }
        int node_count = nodes.size() - 1;
        int matrix_size = node_count + voltage_source_count;
        map<string, int> node_map;
        int current_idx = 0;
        for (auto node : nodes) {
            if (node != GN) node_map[node->getName()] = current_idx++;
        }
        for (auto comp : Components) {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource") {
                node_map[comp->getName()] = current_idx++;
            }
        }

        map<string, vector<double>> results;
        vector<double> time_points;

        for (double time = t_start; time <= t_stop; time += t_step)
        {
            vector<double> x = solve_system(matrix_size, node_map, time, t_step);
            if (x.empty())
            { cout << "Error: Analysis failed (singular matrix or no convergence)." << endl; return; }

            time_points.push_back(time);
            for (const auto& var : vars_to_print)
            {
                results[var].push_back(get_printable_value(var, node_map, x, t_step));
            }

            for (auto const& pair : node_map) {
                if (Node* n = findnode(pair.first))
                {
                    n->setVoltage(x[pair.second]);
                }
            }

            for (auto comp : Components) {
                double current_for_inductor = 0.0;
                if (comp->getType() == "Inductor") {
                    current_for_inductor = get_printable_value("I(" + comp->getName() + ")", node_map, x, t_step);
                }
                comp->update_state(current_for_inductor);
            }
        }


        for (const auto& var : vars_to_print) {
            cout  << var << "   ";
            for(int i = 11-var.length(); i>0; i-- )
            {
                cout<<" ";
            }
        }
        cout << endl;

        for (size_t i = 0; i < time_points.size(); ++i) {

            for (const auto& var : vars_to_print) {
                cout << fixed << setprecision(6) << results[var][i]<< "      " ;
            }
            cout << endl;
        }
    }

    void performDCSweepAnalysis(const string& source_name, double start_v, double end_v, double increment, const string& var_to_print)
    {
        if (GN == nullptr)
        { cout << "Error: No Ground node detected in the circuit." << endl;
            return;
        }

        Component* sweep_comp = findComponent(source_name);
        if (!sweep_comp || sweep_comp->getType() != "DcVoltageSource") {
            cout << "Error: DC sweep source '" << source_name << "' not found or not a DC source." << endl;
            return;
        }

        int voltage_source_count = 0;
        for (auto comp : Components) {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource") {
                voltage_source_count++;
            }
        }
        int node_count = nodes.size() - 1;
        int matrix_size = node_count + voltage_source_count;
        map<string, int> node_map;
        int current_idx = 0;
        for (auto node : nodes)
        {
            if (node != GN) node_map[node->getName()] = current_idx++;
        }
        for (auto comp : Components) {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource")
            {
                node_map[comp->getName()] = current_idx++;
            }
        }

        cout << source_name << "    " << var_to_print << endl;

        for (double v = start_v; v <= end_v; v += increment) {
            sweep_comp->setValue(v);

            vector<double> x = solve_system(matrix_size, node_map, 0, 0);
            if (x.empty()) { cout << "Error: Analysis failed at V=" << v << " (singular matrix or no convergence)." << endl; return; }

            cout << fixed << setprecision(6) << v << "\t" << fixed << setprecision(6) << get_printable_value(var_to_print, node_map, x) << endl;
        }
    }


    double convertToOhms(const string& value)
    {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(k|M|Ω|Meg)?\s*)", regex::icase);
        smatch match;

        if (regex_match(value, match, pattern))
        {
            double number = stod(match[1].str());
            string unit = match[2].str();


            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
            if (unit == "k")
            {
                return number * 1000;
            }
            else if (unit == "m" || unit == "meg")
            {
                return number * 1000000;
            }
            else
            {
                return number;
            }
        }
        else
        {
            return -1;
        }
    }

    double convertToFarad(const string& value) {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(\S*)\s*)", regex::icase);
        smatch match;

        if (regex_match(value, match, pattern)) {
            double number = stod(match[1].str());
            string unit = match[2].str();


            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);

            if (unit == "" || unit == "f") {
                return number;
            }
            else if (unit == "n" || unit == "nf")
            {
                return number * 1e-9;
            }
            else if (unit == "u" || unit == "uf")
            {
                return number * 1e-6;
            }
            else {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    double convertTosecond(const string& value) {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(\S*)\s*)", regex::icase);
        smatch match;

        if (regex_match(value, match, pattern)) {
            double number = stod(match[1].str());
            string unit = match[2].str();


            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
            if (unit == "" || unit == "s") {
                return number;
            }
            else if (unit == "m" || unit == "ms")
            {
                return number * 1e-3;
            }
            else if (unit == "u" || unit == "us")
            {
                return number * 1e-6;
            }
            else
            {
                return stod(value);
            }
        }

    }
    double convertToHenry(const string& value) {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(\S*)\s*)", regex::icase);
        smatch match;

        if (regex_match(value, match, pattern)) {
            double number = stod(match[1].str());
            string unit = match[2].str();


            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
            if (unit == "" || unit == "h") {
                return number;
            }
            else if (unit == "m" || unit == "mh")
            {
                return number * 1e-3;
            }
            else if (unit == "u" || unit == "uh")
            {
                return number * 1e-6;
            }
            else
            {
                return -1;
            }
        }
        else {
            return -1;
        }
    }
    double convert_to_volts(const string& value) {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(V|mV|uV)?\s*)", regex::icase);
        smatch match;
        if (regex_match(value, match, pattern)) {
            double number = stod(match[1].str());
            string unit = match[2].str();
            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
            if (unit == "" || unit == "v") {
                return number;
            }
            else if (unit == "m" || unit == "mv")
            {
                return number * 1e-3;
            }
            else if (unit == "u" || unit == "uv")
            {
                return number * 1e-6;
            }
            else {
                return -1.0;
            }
        }
        else {
            return -1.0;
        }
    }

    double convert_to_amps(const string& value)
    {
        regex pattern(R"(\s*(\d+(?:\.\d+)?)\s*(A|mA|uA)?\s*)", regex::icase);
        smatch match;
        if (regex_match(value, match, pattern))
        {
            double number = stod(match[1].str());
            string unit = match[2].str();
            transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
            if (unit == "" || unit == "a") return number;
            else if (unit == "m" || unit == "ma") return number * 1e-3;
            else if (unit == "u" || unit == "ua") return number * 1e-6;
            else return -1.0;
        }
        else
        {
            return -1.0;
        }
    }
    void add_resistor (string typeName,string node1,string node2,string value)
    {

        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++);
            nodes.push_back(n1);
        }

        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++);
            nodes.push_back(n2);
        }

        if(findComponent(typeName)!= nullptr)
        {
            return;

        }

        double resistance_in_ohms = convertToOhms(value);



        if (resistance_in_ohms != -1)
        {
        }
        else if (resistance_in_ohms == -1)
        {

            cout << "Error: Syntax error"<< endl;
            return;

        }
        if(resistance_in_ohms<= 0)
        {
            cout << "Error: Resistance cannot be zero or negative"<< endl;
            return;

        }
        Resistor* resistor = new Resistor(typeName, n1, n2, resistance_in_ohms);
        Components.push_back(resistor);

    }
    void add_capacitor (string typeName,string node1,string node2,string value)
    {

        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++);
            nodes.push_back(n1);
        }

        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++);
            nodes.push_back(n2);
        }
        if(findComponent(typeName)!= nullptr)
        {
            return;

        }

        double capacitance_in_Farad = convertToFarad(value);



        if (capacitance_in_Farad != -1)
        {
        }
        else if (capacitance_in_Farad == -1)
        {

            cout << "Error: Syntax error"<< endl;
            return;

        }
        if(capacitance_in_Farad<=0)
        {
            cout << "Error: Capacitor cannot be zero or negative"<< endl;
            return;
        }
        Capacitor* capacitor = new Capacitor(typeName, n1, n2, capacitance_in_Farad);
        Components.push_back(capacitor);


    }
    void add_inductor (string typeName,string node1,string node2,string value)
    {

        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++);
            nodes.push_back(n1);
        }

        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++);
            nodes.push_back(n2);
        }
        if(findComponent(typeName)!= nullptr)
        {
            return;

        }

        double inductance_in_Henry = convertToHenry(value);



        if (inductance_in_Henry != -1)
        {
        }
        else if (inductance_in_Henry == -1)
        {

            cout << "Error: Syntax error"<< endl;

            return;

        }
        if(inductance_in_Henry<=0)
        {
            cout << "Error: Inductor cannot be zero or negative"<< endl;

            return;
        }
        Inductor* inductor = new Inductor(typeName, n1, n2, inductance_in_Henry);
        Components.push_back(inductor);


    }
    void add_diode(string typeName, string node1, string node2, string model)
    {
        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++);
            nodes.push_back(n1);
        }

        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++);
            nodes.push_back(n2);
        }

        if (findComponent(typeName) != nullptr)
        {
            return;
        }

        if (model != "D" && model != "Z") {
            cout << "Error: Model " << model << " not found in library" << endl;
            return;
        }
        double ZENERV = 0.0;

        if (model == "D") {

        }
        else if (model == "Z")
        {
            ZENERV = 0.7;
        }
        else {
            cout << "Error: Model " << model << " not found in library" << endl;
            return;
        }

        Diode* diode = new Diode(typeName, n1, n2, model, ZENERV);
        Components.push_back(diode);
    }
    void add_dc_voltage_source(string typeName, string node1, string node2, string value_str) {
        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++); nodes.push_back(n1);
        }
        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++); nodes.push_back(n2);
        }

        if(findComponent(typeName)!= nullptr) {
            return;
        }

        double voltage_val = convert_to_volts(value_str);
        if (voltage_val == -1) {
            cout << "Error: Syntax error in DC voltage value." << endl; return;
        }

        DcVoltageSource* vs = new DcVoltageSource(typeName, n1, n2, voltage_val);
        Components.push_back(vs);
    }

    void add_sin_voltage_source(string typeName, string node1, string node2, string offset_string, string amp_string, string frequency_str) {
        Node* n1 = findnode(node1);
        if (!n1) { n1 = new Node(node1, Node_count++); nodes.push_back(n1); }
        Node* n2 = findnode(node2);
        if (!n2) { n2 = new Node(node2, Node_count++); nodes.push_back(n2); }

        if(findComponent(typeName)!= nullptr) { return; }

        double voffset = convert_to_volts(offset_string);
        double vamplitude = convert_to_volts(amp_string);
        double frequency = stod(frequency_str);

        if (voffset == -1 || vamplitude == -1)
        {
            cout << "Error: Syntax error." << endl;
            return;
        }
        if (frequency <= 0) {
            cout << "Error: Frequency must be positive." << endl; return;
        }

        SinVoltageSource* sin_vs = new SinVoltageSource(typeName, n1, n2, voffset, vamplitude, frequency);
        Components.push_back(sin_vs);
    }
    void add_current_source(string typeName, string node1, string node2, string value) {
        Node* n1 = findnode(node1);
        if (!n1) {
            n1 = new Node(node1, Node_count++); nodes.push_back(n1);
        }
        Node* n2 = findnode(node2);
        if (!n2) {
            n2 = new Node(node2, Node_count++); nodes.push_back(n2);
        }

        if (findComponent(typeName)!= nullptr) {
            return; }

        double current_val = convert_to_amps(value);
        if (current_val == -1) {
            cout << "Error: Syntax error." << endl; return;
        }
        CurrentSource* cs = new CurrentSource(typeName, n1, n2, current_val);
        Components.push_back(cs);
    }

    void deleteDiode(string name)
    {
        auto D1 = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name && comp->getType() == "Diode";
        });

        if (D1 != Components.end())
        {
            delete *D1;
            Components.erase(D1);
            cout << "Diode " << name << " deleted successfully." << endl;
        }
        else {
            cout << "Error: Cannot delete diode; component not found" << endl;
        }
    }
    void deleteNode(string name)
    {
        auto it = find_if(nodes.begin(), nodes.end(), [&name](Node* node) { return node->getName() == name; });

        if (it != nodes.end())
        {
            delete *it;
            nodes.erase(it);
            cout << "Node " << name << " deleted successfully." << endl;
        }
        else
        {
            cout << "Error: Node not found!" << endl;
        }
    }
    void deleteResistor(string name)
    {
        auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name && comp->getType() == "Resistor";
        });

        if (it != Components.end())
        {
            delete *it;
            Components.erase(it);
            cout << "Resistor " << name << " deleted successfully." << endl;
        } else {
            cout << "Error: Cannot delete resistor; component not found" << endl;
        }
    }
    void deleteCapacitor(string name)
    {
        auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name && comp->getType() == "Capacitor";
        });

        if (it != Components.end()) {
            delete *it;
            Components.erase(it);
            cout << "Capacitor " << name << " deleted successfully." << endl;
        }
        else {
            cout << "Error: Cannot delete Capacitor; component not found" << endl;
        }
    }
    void deleteInductor(string name) {
        auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name && comp->getType() == "Inductor";
        });

        if (it != Components.end())
        {
            delete *it;
            Components.erase(it);
            cout << "Inductor " << name << " deleted successfully." << endl;
        }
        else {
            cout << "Error: Cannot delete Inductor; component not found" << endl;
        }
    }
    void deleteVoltageSource(string name) {
        auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name &&
                   (comp->getType() == "DcVoltageSource" ||
                    comp->getType() == "SinVoltageSource" ||
                    comp->getType() == "WaveformSource");
        });

        if (it != Components.end()) {
            delete *it;
            Components.erase(it);
            cout << "Voltage source " << name << " deleted successfully." << endl;
        } else {
            cout << "Error: Cannot delete voltage source; component not found" << endl;
        }
    }

    void deleteCurrentSource(string name) {
        auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
            return comp->getName() == name && comp->getType() == "CurrentSource";
        });

        if (it != Components.end()) {
            delete *it;
            Components.erase(it);
            cout << "Current source " << name << " deleted successfully." << endl;
        } else {
            cout << "Error: Cannot delete current source; component not found" << endl;
        }
    }
    void add_ground_node(string nodeName)
    {
        if (GN != nullptr) {
            cout << "Error: Only one ground node is allowed." << endl;
            return;
        }


        Node* existingNode = findnode(nodeName);
        if (existingNode == nullptr) {

            GN = new Node(nodeName, Node_count++);
            GN->setVoltage(0.0);
            nodes.push_back(GN);
        }
        else {

            GN = existingNode;
            GN->setVoltage(0.0);
        }
    }

    void delete_ground_node(string nodeName)
    {
        if (GN == nullptr || GN->getName() != nodeName) {
            cout << "Error: Node " << nodeName << " is not the ground node or does not exist." << endl;

            auto it = find_if(nodes.begin(), nodes.end(), [&](Node* node) {
                return node->getName() == nodeName; });
            if (it == nodes.end()) {
                cout << "Error: Node " << nodeName << " does not exist." << endl;
            }
            else {
                cout << "Error: Node " << nodeName << " is not the designated ground node." << endl;
            }
            return;
        }
        delete GN;
        nodes.erase(remove(nodes.begin(), nodes.end(), GN), nodes.end());
        GN = nullptr;
    }
    void show_nodes() {
        if (nodes.empty())
        {
            cout << "No nodes in the circuit." << endl;
            return;
        }
        cout << "Available nodes:" << endl;
        cout<<"Name:"<<nodes[0]->getName()<<" Voltage:"<<nodes[0]->getVoltage();
        for ( int i =1; i<nodes.size(); i++)
        {


            cout<<" ,Name:"<<nodes[i]->getName()<<" Voltage:"<<nodes[i]->getVoltage();

        }
        cout << endl;
    }
    void show_all() {
        if (Components.empty())
        {
            cout << "No Components in the circuit." << endl;
            return;
        }
        cout << "Available Components:" << endl;

        for ( int i =0; i<Components.size(); i++)
        {

            cout<<"Name: "<<Components[i]->getName()<<" ,Type: "<<Components[i]->getType()<<" ,Nodes: "<<Components[i]->getNode1()->getName()<<" - "<<Components[i]->getNode2()->getName()<<" ,Value: "<<Components[i]->getvalue()<<endl;

        }
        cout << endl;
    }
    void showbytype (string type)
    {

        bool first = false;
        if(Components.empty())
        {
            cout << "No components of type " << type << " found." << endl;

        }
        else
        {

            for (Component* comp : Components)
            {


                if (comp->getType() == type)
                {
                    if(!first)
                    {
                        cout << "Available "<<comp->getType()<<":" << endl;
                    }



                    cout<<"Name: "<<comp->getName()<<" ,Nodes: "<<comp->getNode1()->getName()<<" - "<<comp->getNode2()->getName()<<" ,Value: "<<comp->getvalue()<<endl;
                    first = true;



                }
            }
        }
    }
    string formatValue(Component* comp) {
        double val = comp->getvalue();
        string type = comp->getType();
        string unit = "";

        if (type == "Resistor") unit = " Ohm";
        else if (type == "Capacitor") unit = "F";
        else if (type == "Inductor") unit = "H";
        else if (type == "DcVoltageSource" || type == "SinVoltageSource") unit = "V";
        else if (type == "CurrentSource") unit = "A";
        else if (type == "Diode") return "";

        stringstream ss;

        if (abs(val) >= 1e9) {
            ss << fixed << setprecision(2) << (val / 1e9) << "G" << unit;
        } else if (abs(val) >= 1e6) {
            ss << fixed << setprecision(2) << (val / 1e6) << "M" << unit;
        } else if (abs(val) >= 1e3) {
            ss << fixed << setprecision(2) << (val / 1e3) << "k" << unit;
        } else if (abs(val) < 1e-9) {
            ss << fixed << setprecision(2) << (val * 1e12) << "p" << unit;
        } else if (abs(val) < 1e-6) {
            ss << fixed << setprecision(2) << (val * 1e9) << "n" << unit;
        } else if (abs(val) < 1e-3) {
            ss << fixed << setprecision(2) << (val * 1e6) << "u" << unit;
        } else if (abs(val) < 1) {
            ss << fixed << setprecision(2) << (val * 1e3) << "m" << unit;
        } else {
            ss << fixed << setprecision(2) << val << unit;
        }
        return ss.str();
    }

    bool checkGnd (string name)
    {
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        if(name=="gnd")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    void renamenode ( string oldname, string newname)
    {
        Node* node_to_rename = findnode(oldname);


        if (!node_to_rename)
        {
            cout << "ERROR: Node " << oldname << " does not exist in the circuit" << endl;
            return;
        }

        if (oldname == newname)
        {

            cout << "SUCCESS: Node renamed from " << oldname << " to " << newname << endl;
            return;
        }

        if (findnode(newname) != nullptr) {
            cout << "ERROR: Node name " << newname << " already exists" << endl;
            return;
        }


        node_to_rename->setName(newname);


        cout << "SUCCESS: Node renamed from " << oldname << " to " << newname << endl;
    }


    //GRAPHIC PART:
    // این همان تابع قبلی خودمونه که اینجا صرفا خروجی رو به جای چاپ تو کنسول داخل یه مپ ذخیره میکنیم تا بعدا نشونش بدیم

    map<string, vector<double>> runTransientAnalysis(double t_step, double t_stop, double t_start, const vector<string>& vars_to_print, vector<double>& time_points)
    {
        map<string, vector<double>> results;
        time_points.clear();

        if (GN == nullptr)
        {
            cout << "Error: GND node not set." << endl;
            return results;
        }

        int voltage_source_count = 0;
        for (auto comp : Components)

        {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource" || comp->getType() == "WaveformSource") {
                voltage_source_count++;
            }
        }
        int node_count = nodes.size() - 1;
        int matrix_size = node_count + voltage_source_count;
        map<string, int> node_map;
        int current_idx = 0;
        for (auto node : nodes) {
            if (node != GN) node_map[node->getName()] = current_idx++;
        }
        for (auto comp : Components) {
            if (comp->getType() == "DcVoltageSource" || comp->getType() == "SinVoltageSource" || comp->getType() == "WaveformSource") {
                node_map[comp->getName()] = current_idx++;
            }
        }

        for (double time = t_start; time <= t_stop; time += t_step)
        {
            vector<double> x = solve_system(matrix_size, node_map, time, t_step);
            if (x.empty())
            {
                cout << "Error: Analysis failed (singular matrix or no convergence)." << endl;
                results.clear(); // Clear any partial results
                return results;
            }

            time_points.push_back(time);
            for (const auto& var : vars_to_print)
            {
                results[var].push_back(get_printable_value(var, node_map, x, t_step));
            }

            for (auto const& pair : node_map) {
                if (Node* n = findnode(pair.first))
                {
                    n->setVoltage(x[pair.second]);
                }
            }

            for (auto comp : Components) {
                double current_for_inductor = 0.0;
                if (comp->getType() == "Inductor") {
                    current_for_inductor = get_printable_value("I(" + comp->getName() + ")", node_map, x, t_step);
                }
                comp->update_state(current_for_inductor);
            }
        }
        return results;
    }

};



const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int FASELE_NOGHAT = 20;


class InputDialog {
public:
    bool active;
    Component* target;
    int active_field;

    vector<string> labels;
    vector<string> values;

    SDL_Rect bg_rect;
    vector<SDL_Rect> field_rects;
    SDL_Rect ok_button_rect;
    SDL_Rect cancel_button_rect;

    InputDialog() : active(false), target(nullptr), active_field(0) {}

    void setup(Component* component, int mouseX, int mouseY) {
        active = true;
        target = component;
        active_field = 0;
        labels.clear();
        values.clear();

        string type = component->getType();

        if (type == "Resistor") {
            labels = {"Name:", "Resistance (e.g. 1k):"};
            values = {component->getName(), "1k"};
        }
        else if (type == "Capacitor") {
            labels = {"Name:", "Capacitance (e.g. 1uF):"};
            values = {component->getName(), "1uF"};
        }
        else if (type == "Inductor") {
            labels = {"Name:", "Inductance (e.g. 1mH):"};
            values = {component->getName(), "1mH"};
        }
        else if (type == "Diode") {
            labels = {"Name:", "Model (D or Z):"};
            values = {component->getName(), "D"};
        }
        else if (type == "DcVoltageSource") {
            labels = {"Name:", "Voltage (e.g. 5V):"};
            values = {component->getName(), "5V"};
        }
        else if (type == "CurrentSource") {
            labels = {"Name:", "Current (e.g. 1A):"};
            values = {component->getName(), "1A"};
        }
        else if (type == "SinVoltageSource") {
            labels = {"Name:", "Offset (V):", "Amplitude (V):", "Frequency (Hz):"};
            values = {component->getName(), "0", "5", "1000"};
        }

        int dialog_w = 350;
        int dialog_h = 50 + (values.size() * 40) + 60;
        bg_rect = {mouseX, mouseY, dialog_w, dialog_h};

        if (bg_rect.x + bg_rect.w > SCREEN_WIDTH) bg_rect.x = SCREEN_WIDTH - bg_rect.w;
        if (bg_rect.y + bg_rect.h > SCREEN_HEIGHT) bg_rect.y = SCREEN_HEIGHT - bg_rect.h;


        field_rects.clear();
        for (size_t i = 0; i < values.size(); ++i) {
            field_rects.push_back({bg_rect.x + 150, bg_rect.y + 50 + (int)i * 40, 180, 30});
        }

        ok_button_rect = {bg_rect.x + (dialog_w / 2) - 110, bg_rect.y + dialog_h - 45, 100, 30};
        cancel_button_rect = {bg_rect.x + (dialog_w / 2) + 10, bg_rect.y + dialog_h - 45, 100, 30};

        SDL_StartTextInput();
    }

    void close() {
        active = false;
        target = nullptr;
        SDL_StopTextInput();
    }
};


//GRID
// نقطه چین درست میکنه این تو صفحه، اینو زدم که سیمکشی و اینا راحت باشه
void draw_grid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    for (int x = 0; x < SCREEN_WIDTH; x += FASELE_NOGHAT) {
        for (int y = 0; y < SCREEN_HEIGHT; y += FASELE_NOGHAT) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}
//تابع ساده مستطیل توپر
void FilledRect (SDL_Renderer* ren, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
    SDL_Rect rect{ x, y, w, h };
    SDL_SetRenderDrawColor(ren, r, g, b, a);
    SDL_RenderFillRect(ren, &rect);
}

//این تابع برای اشکال منو هست و همچنین آیکونهای دیگه

void drawNewFileIcon(SDL_Renderer* renderer, int x, int y, int size) {
    Uint8 page_r = 0,   page_g = 0,   page_b = 0,   page_a = 255;
    Uint8 circle_r = 0, circle_g = 0, circle_b = 255, circle_a = 255;
    Uint8 plus_r = 255, plus_g = 255, plus_b = 255, plus_a = 255;

    int thickness = size / 12;
    if (thickness < 2) thickness = 2;

    int corner_size = size / 4;
    Sint16 page_vx[] = {
            (Sint16)x, (Sint16)(x + size - corner_size), (Sint16)(x + size), (Sint16)(x + size), (Sint16)x
    };
    Sint16 page_vy[] = {
            (Sint16)y, (Sint16)y, (Sint16)(y + corner_size), (Sint16)(y + size), (Sint16)(y + size)
    };
    filledPolygonRGBA(renderer, page_vx, page_vy, 5, page_r, page_g, page_b, page_a);

    lineRGBA(renderer, x + size - corner_size, y, x + size - corner_size, y + corner_size, page_r, page_g, page_b, page_a);
    lineRGBA(renderer, x + size - corner_size, y + corner_size, x + size, y + corner_size, page_r, page_g, page_b, page_a);

    int circle_radius = size / 3;
    int circle_cx = x + circle_radius;
    int circle_cy = y + size - circle_radius;
    filledCircleRGBA(renderer, circle_cx, circle_cy, circle_radius, circle_r, circle_g, circle_b, circle_a);
    circleRGBA(renderer, circle_cx, circle_cy, circle_radius, 0, 0, 0, 255);

    int plus_size = circle_radius * 0.6;
    thickLineRGBA(renderer,
                  circle_cx - plus_size, circle_cy,
                  circle_cx + plus_size, circle_cy,
                  thickness, plus_r, plus_g, plus_b, plus_a);
    thickLineRGBA(renderer,
                  circle_cx, circle_cy - plus_size,
                  circle_cx, circle_cy + plus_size,
                  thickness, plus_r, plus_g, plus_b, plus_a);
}

void drawFolderShape(SDL_Renderer* renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int tab_width = width / 3;
    int tab_height = height / 5;

    Sint16 folder_vx[] = {
            (Sint16)x, (Sint16)(x + tab_width), (Sint16)(x + tab_width), (Sint16)(x + width), (Sint16)(x + width), (Sint16)x
    };
    Sint16 folder_vy[] = {
            (Sint16)(y + tab_height), (Sint16)(y + tab_height), (Sint16)y, (Sint16)y, (Sint16)(y + height), (Sint16)(y + height)
    };

    filledPolygonRGBA(renderer, folder_vx, folder_vy, 6, r, g, b, a);
}

void drawOpenFileIcon(SDL_Renderer* renderer, int x, int y, int size) {
    Uint8 black[] = {20, 20, 20, 255};
    Uint8 blue[]  = {0, 80, 255, 255};
    Uint8 white[] = {255, 255, 255, 255};
    int width = size;
    int height = size * 0.75;
    drawFolderShape(renderer, x, y, width, height, black[0], black[1], black[2], black[3]);
    int front_folder_y = y + height / 4;
    drawFolderShape(renderer, x, front_folder_y, width, height, blue[0], blue[1], blue[2], blue[3]);
    int arrow_height = height * 0.4;
    int arrow_width = arrow_height * 0.7;
    int arrow_cx = x + width / 2;
    int arrow_cy = front_folder_y + height / 2 + arrow_height * 0.1;
    int thickness = size / 12;
    if (thickness < 3) {
        thickness = 3;
    }

    boxRGBA(renderer,
            arrow_cx - thickness / 2, arrow_cy,
            arrow_cx + thickness / 2, arrow_cy + arrow_height / 2,
            white[0], white[1], white[2], white[3]);

    Sint16 arrow_head_vx[] = { (Sint16)(arrow_cx - arrow_width/2), (Sint16)(arrow_cx + arrow_width/2), (Sint16)arrow_cx };
    Sint16 arrow_head_vy[] = { (Sint16)arrow_cy, (Sint16)arrow_cy, (Sint16)(arrow_cy - arrow_height / 2) };
    filledPolygonRGBA(renderer, arrow_head_vx, arrow_head_vy, 3, white[0], white[1], white[2], white[3]);
}

void drawSaveIcon(SDL_Renderer* renderer, int x, int y, int size) {
    Uint8 black[] = {20, 20, 20, 255};
    Uint8 blue[]  = {0, 80, 255, 255};
    Uint8 white[] = {255, 255, 255, 255};

    int corner_cut = size / 4;
    Sint16 body_vx[] = {
            (Sint16)x, (Sint16)(x + size - corner_cut), (Sint16)(x + size), (Sint16)(x + size), (Sint16)x
    };
    Sint16 body_vy[] = {
            (Sint16)y, (Sint16)y, (Sint16)(y + corner_cut), (Sint16)(y + size), (Sint16)(y + size)
    };
    filledPolygonRGBA(renderer, body_vx, body_vy, 5, black[0], black[1], black[2], black[3]);

    int shutter_h = size / 3;
    int shutter_w = size * 0.6;
    int shutter_x = x + (size - shutter_w) / 2;
    int shutter_y = y + size - shutter_h;
    boxRGBA(renderer, shutter_x, shutter_y, shutter_x + shutter_w, shutter_y + shutter_h, white[0], white[1], white[2], white[3]);

    int label_h = size / 6;
    boxRGBA(renderer, shutter_x, y + size - label_h, shutter_x + shutter_w, y + size, blue[0], blue[1], blue[2], blue[3]);

    int detail_w = size / 4;
    int detail_h = size / 5;
    int detail_x = x + size / 4;
    int detail_y = y;
    boxRGBA(renderer, detail_x, detail_y, detail_x + detail_w, detail_y + detail_h, black[0], black[1], black[2], black[3]);
}

void drawPlayIcon(SDL_Renderer* renderer, int x, int y, int size) {
    Uint8 fill_r = 0,   fill_g = 220, fill_b = 0,   fill_a = 255;
    Uint8 border_r = 0, border_g = 0,   border_b = 0,   border_a = 255;

    int thickness = size / 10;
    if (thickness < 2) thickness = 2;

    Sint16 vx[] = {
            (Sint16)x,
            (Sint16)x,
            (Sint16)(x + size * 0.866)
    };
    Sint16 vy[] = {
            (Sint16)y,
            (Sint16)(y + size),
            (Sint16)(y + size / 2)
    };

    filledPolygonRGBA(renderer, vx, vy, 3, fill_r, fill_g, fill_b, fill_a);

    for (int i = 0; i < 3; ++i) {
        thickLineRGBA(renderer,
                      vx[i], vy[i],
                      vx[(i + 1) % 3], vy[(i + 1) % 3],
                      thickness,
                      border_r, border_g, border_b, border_a);
    }
}

void drawComponentIcon(SDL_Renderer* renderer, int x, int y, int width, int height) {
    Uint8 body_c[] = {20, 20, 20, 255};
    Uint8 pin_c[]  = {0, 80, 255, 255};
    Uint8 play_c[] = {255, 255, 255, 255};
    Uint8 notch_c[] = {211, 211, 211, 255};

    int pin_length = width / 5;
    int pin_height = height / 6;

    for (int i = 0; i < 3; ++i) {
        int pin_y = y + (height / 6) + (i * height / 3);

        boxRGBA(renderer, x - pin_length, pin_y - pin_height / 2, x, pin_y + pin_height / 2, pin_c[0], pin_c[1], pin_c[2], pin_c[3]);
        filledCircleRGBA(renderer, x - pin_length, pin_y, pin_height / 2, pin_c[0], pin_c[1], pin_c[2], pin_c[3]);

        boxRGBA(renderer, x + width, pin_y - pin_height / 2, x + width + pin_length, pin_y + pin_height / 2, pin_c[0], pin_c[1], pin_c[2], pin_c[3]);
        filledCircleRGBA(renderer, x + width + pin_length, pin_y, pin_height/2, pin_c[0], pin_c[1], pin_c[2], pin_c[3]);
    }

    boxRGBA(renderer, x, y, x + width, y + height, body_c[0], body_c[1], body_c[2], body_c[3]);

    int notch_w = width / 4;
    boxRGBA(renderer, x + (width - notch_w) / 2, y, x + (width + notch_w) / 2, y + height / 10, notch_c[0], notch_c[1], notch_c[2], notch_c[3]);

    int triangle_h = height / 3;
    int triangle_w = triangle_h * 0.866;
    int triangle_x = x + (width - triangle_w) / 2;
    int triangle_y = y + (height - triangle_h) / 2;

    Sint16 vx[] = {(Sint16)triangle_x, (Sint16)triangle_x, (Sint16)(triangle_x + triangle_w)};
    Sint16 vy[] = {(Sint16)triangle_y, (Sint16)(triangle_y + triangle_h), (Sint16)(triangle_y + triangle_h / 2)};
    filledPolygonRGBA(renderer, vx, vy, 3, play_c[0], play_c[1], play_c[2], play_c[3]);
}

void drawResistorIcon(SDL_Renderer* renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_Point points[] = {
            {x, y},
            {(int)(x + width * 0.15), y},
            {(int)(x + width * 0.25), y - height},
            {(int)(x + width * 0.45), y + height},
            {(int)(x + width * 0.65), y - height},
            {(int)(x + width * 0.85), y + height},
            {(int)(x + width * 0.95), y - height},
            {(int)(x + width*1.05), y},
            {(int)(x + width *1.25), y}
    };

    int num_points = sizeof(points) / sizeof(points[0]);

    for (int i = 0; i < num_points - 1; ++i) {
        thickLineRGBA(renderer,
                      points[i].x, points[i].y,
                      points[i+1].x, points[i+1].y,
                      3,
                      r, g, b, a);
    }
}


void drawCapacitorIcon(SDL_Renderer* renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int lead_width = width * 0.4;
    int gap = width * 0.2;
    int y_center = y + height / 2;

    thickLineRGBA(renderer,
                  x, y_center,
                  x + lead_width, y_center,
                  3, r, g, b, a);

    thickLineRGBA(renderer,
                  x + lead_width, y,
                  x + lead_width, y + height,
                  3, r, g, b, a);

    thickLineRGBA(renderer,
                  x + lead_width + gap, y,
                  x + lead_width + gap, y + height,
                  3, r, g, b, a);

    thickLineRGBA(renderer,
                  x + lead_width + gap, y_center,
                  x + width, y_center,
                  3, r, g, b, a);
}


void drawInductorIcon_Rounded(SDL_Renderer* renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;
    std::vector<SDL_Point> points;

    double lead_width = width * 0.15;
    points.push_back({x, y});
    points.push_back({(int)(x + lead_width), y});

    double coil_section_width = width * 0.7;
    int num_loops = 4;
    double loop_width = coil_section_width / num_loops;
    double radius = loop_width / 2.0;
    int segments_per_loop = 12;

    for (int i = 0; i < num_loops; ++i) {
        double loop_center_x = (x + lead_width) + (i * loop_width) + radius;

        for (int j = 0; j <= segments_per_loop; ++j) {
            double angle = M_PI - (M_PI * j / segments_per_loop);
            double point_x = loop_center_x + radius * cos(angle);
            double point_y = y - radius * sin(angle);
            points.push_back({(int)point_x, (int)point_y});
        }
    }

    points.push_back({x + width, y});

    for (size_t i = 0; i < points.size() - 1; ++i) {
        thickLineRGBA(renderer,
                      points[i].x, points[i].y,
                      points[i+1].x, points[i+1].y,
                      thickness, r, g, b, a);
    }
}


void drawDiodeIcon(SDL_Renderer* renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;
    int y_center = y + height / 2;

    double lead_width = width * 0.3;
    double triangle_width = width * 0.4;

    int x_start_triangle = x + lead_width;
    int x_end_triangle = x_start_triangle + triangle_width;

    thickLineRGBA(renderer, x, y_center, x_start_triangle, y_center, thickness, r, g, b, a);

    Sint16 triangle_vx[] = { (Sint16)x_start_triangle, (Sint16)x_start_triangle, (Sint16)x_end_triangle };
    Sint16 triangle_vy[] = { (Sint16)y, (Sint16)(y + height), (Sint16)y_center };
    filledPolygonRGBA(renderer, triangle_vx, triangle_vy, 3, r, g, b, a);

    thickLineRGBA(renderer, x_end_triangle, y, x_end_triangle, y + height, thickness, r, g, b, a);

    thickLineRGBA(renderer, x_end_triangle, y_center, x + width, y_center, thickness, r, g, b, a);
}


void drawCurrentSourceIcon(SDL_Renderer* renderer, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;

    for (int i = 0; i < thickness; ++i) {
        circleRGBA(renderer, cx, cy, radius - i, r, g, b, a);
    }

    int lead_length = radius / 2;
    thickLineRGBA(renderer, cx, cy - radius, cx, cy - radius - lead_length, thickness, r, g, b, a);
    thickLineRGBA(renderer, cx, cy + radius, cx, cy + radius + lead_length, thickness, r, g, b, a);

    int arrow_length = radius * 0.8;
    int arrow_y_start = cy + arrow_length / 2;
    int arrow_y_end = cy - arrow_length / 2;

    thickLineRGBA(renderer, cx, arrow_y_start, cx, arrow_y_end, thickness, r, g, b, a);

    int arrowhead_size = radius * 0.3;
    thickLineRGBA(renderer, cx, arrow_y_end, cx - arrowhead_size, arrow_y_end + arrowhead_size, thickness, r, g, b, a);
    thickLineRGBA(renderer, cx, arrow_y_end, cx + arrowhead_size, arrow_y_end + arrowhead_size, thickness, r, g, b, a);
}


void drawSineVoltageSourceIcon(SDL_Renderer* renderer, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;

    for (int i = 0; i < thickness; ++i) {
        circleRGBA(renderer, cx, cy, radius - i, r, g, b, a);
    }

    int lead_length = radius / 2;
    thickLineRGBA(renderer, cx, cy - radius, cx, cy - radius - lead_length, thickness, r, g, b, a);
    thickLineRGBA(renderer, cx, cy + radius, cx, cy + radius + lead_length, thickness, r, g, b, a);

    std::vector<SDL_Point> points;
    double sine_width = radius * 1.4;
    double amplitude = radius * 0.4;
    int num_segments = 30;
    double num_cycles = 1.5;

    double start_x = cx - sine_width / 2.0;

    for (int i = 0; i <= num_segments; ++i) {
        double progress = (double)i / num_segments;
        double point_x = start_x + (progress * sine_width);
        double point_y = cy + (amplitude * sin(progress * M_PI * 2.0 * num_cycles));
        points.push_back({(int)point_x, (int)point_y});
    }

    for (size_t i = 0; i < points.size() - 1; ++i) {
        thickLineRGBA(renderer,
                      points[i].x, points[i].y,
                      points[i+1].x, points[i+1].y,
                      thickness, r, g, b, a);
    }
}


void drawDCVoltageSourceIcon(SDL_Renderer* renderer, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;

    for (int i = 0; i < thickness; ++i) {
        circleRGBA(renderer, cx, cy, radius - i, r, g, b, a);
    }

    int lead_length = radius / 2;
    thickLineRGBA(renderer, cx, cy - radius, cx, cy - radius - lead_length, thickness, r, g, b, a);
    thickLineRGBA(renderer, cx, cy + radius, cx, cy + radius + lead_length, thickness, r, g, b, a);

    int sign_size = radius * 0.4;

    int plus_cy = cy - radius / 2;
    thickLineRGBA(renderer, cx - sign_size / 2, plus_cy, cx + sign_size / 2, plus_cy, thickness, r, g, b, a);
    thickLineRGBA(renderer, cx, plus_cy - sign_size / 2, cx, plus_cy + sign_size / 2, thickness, r, g, b, a);

    int minus_cy = cy + radius / 2;
    thickLineRGBA(renderer, cx - sign_size / 2, minus_cy, cx + sign_size / 2, minus_cy, thickness, r, g, b, a);
}

//تابع رسم سیگناله این
//طبق گفته داک میاد اول رنج تایم و ولتاژ رو درمیاره، سپس از datapointها استفاده میکنه و نقطه رو به نقطه وصل میکنه و نمودار میسازه
void drawSignal(SDL_Renderer* renderer,
                const vector<double>& time,
                const vector<double>& signal,
                const SDL_Rect& plotArea,
                double min_time, double max_time,
                double min_voltage, double max_voltage)
{
    if (time.size() < 2 || signal.size() < 2 || time.size() != signal.size()) {
        return;
    }
    double time_range = max_time - min_time;
    double voltage_range = max_voltage - min_voltage;

    if (time_range == 0) {
        time_range = 1;
    }
    if (voltage_range == 0) {
        voltage_range = 1;
    }

    for (size_t i = 1; i < time.size(); ++i) {
        int x1 = plotArea.x + ((time[i-1] - min_time) / time_range) * plotArea.w;
        int y1 = plotArea.y + plotArea.h - ((signal[i-1] - min_voltage) / voltage_range) * plotArea.h;

        int x2 = plotArea.x + ((time[i] - min_time) / time_range) * plotArea.w;
        int y2 = plotArea.y + plotArea.h - ((signal[i] - min_voltage) / voltage_range) * plotArea.h;

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}
void drawText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y, SDL_Color color) {
    if (font == nullptr) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

//تابع برای کشیدن محورهای نمودار سیگنال
void drawAxes(SDL_Renderer* renderer, TTF_Font* font, const SDL_Rect& plotArea,
              double minTime, double maxTime, double minVoltage, double maxVoltage) {

    SDL_Color axisColor = {255, 255, 255, 255};
    SDL_SetRenderDrawColor(renderer, axisColor.r, axisColor.g, axisColor.b, axisColor.a);

    SDL_RenderDrawLine(renderer, plotArea.x, plotArea.y, plotArea.x, plotArea.y + plotArea.h);
    SDL_RenderDrawLine(renderer, plotArea.x, plotArea.y + plotArea.h, plotArea.x + plotArea.w, plotArea.y + plotArea.h);

    const int num_of_divs = 10;
    const int tickLength = 5;

    double voltageStep = (maxVoltage - minVoltage) / num_of_divs;
    for (int i = 0; i <= num_of_divs; ++i) {
        int y = plotArea.y + (i * plotArea.h / num_of_divs);
        SDL_RenderDrawLine(renderer, plotArea.x, y, plotArea.x - tickLength, y);
        double value = maxVoltage - (i * voltageStep);
        stringstream ss;
        ss << fixed << setprecision(1) << value;

        drawText(renderer, font, ss.str(), plotArea.x - 45, y - 8, axisColor);
    }

    double timeStep = (maxTime - minTime) / num_of_divs;
    for (int i = 0; i <= num_of_divs; ++i) {
        int x = plotArea.x + (i * plotArea.w / num_of_divs);
        // Draw the tick mark line
        SDL_RenderDrawLine(renderer, x, plotArea.y + plotArea.h, x, plotArea.y + plotArea.h + tickLength);

        // Calculate and format the label value
        double value = minTime + (i * timeStep);
        stringstream ss;
        ss << fixed << setprecision(2) << value; // Format to two decimal places

        // Draw the text label
        drawText(renderer, font, ss.str(), x - 10, plotArea.y + plotArea.h + 8, axisColor);
    }
}
//تابع زوم
void zoom(double& viewMin, double& viewMax, double F, double center) {
    double range1 = viewMax - viewMin;
    double range2 = range1 / F;
    viewMin = center - (center - viewMin) * (range2 / range1);
    viewMax = viewMin + range2;
}

// تابع رسم مقاومت که از قبل داشتیم
void drawRotatableResistor(SDL_Renderer* renderer, int x, int y, int length, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int zig_height = 10;
    int num_points = 8; // تعداد نقاط اصلی شکل
    SDL_Point points[num_points];

    points[0] = {0, 0};
    points[1] = {(int)(length * 0.15), 0};
    points[2] = {(int)(length * 0.25), -zig_height};
    points[3] = {(int)(length * 0.45), zig_height};
    points[4] = {(int)(length * 0.65), -zig_height};
    points[5] = {(int)(length * 0.85), 0};
    points[6] = {(int)(length * 0.85), 0}; // این نقطه در واقع سیم انتهایی است
    points[7] = {length, 0};

    double angle = rotation * M_PI / 2.0; // 0, 90, 180, 270 degrees
    SDL_Point rotated_points[num_points];

    for (int i = 0; i < num_points; ++i) {
        int rotated_x = round(points[i].x * cos(angle) - points[i].y * sin(angle));
        int rotated_y = round(points[i].x * sin(angle) + points[i].y * cos(angle));
        rotated_points[i] = {x + rotated_x, y + rotated_y};
    }

    for (int i = 0; i < num_points - 1; ++i) {
        thickLineRGBA(renderer, rotated_points[i].x, rotated_points[i].y, rotated_points[i+1].x, rotated_points[i+1].y, 3, r, g, b, a);
    }
}

// تابع رسم خازن
void drawRotatableCapacitor(SDL_Renderer* renderer, int x, int y, int length, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int plate_height = 30;
    int lead_len = (length - 10) / 2;

    SDL_Point p1 = {0, 0};
    SDL_Point p2 = {lead_len, 0};
    SDL_Point p3 = {lead_len, -plate_height/2};
    SDL_Point p4 = {lead_len, plate_height/2};

    SDL_Point p5 = {lead_len + 10, -plate_height/2};
    SDL_Point p6 = {lead_len + 10, plate_height/2};
    SDL_Point p7 = {lead_len + 10, 0};
    SDL_Point p8 = {length, 0};

    SDL_Point points[] = {p1, p2, p3, p4, p5, p6, p7, p8};
    double angle = rotation * M_PI / 2.0;

    for(auto& pt : points) {
        int rotated_x = round(pt.x * cos(angle) - pt.y * sin(angle));
        int rotated_y = round(pt.x * sin(angle) + pt.y * cos(angle));
        pt = {x + rotated_x, y + rotated_y};
    }
    thickLineRGBA(renderer, points[0].x, points[0].y, points[1].x, points[1].y, 3, r,g,b,a);
    thickLineRGBA(renderer, points[2].x, points[2].y, points[3].x, points[3].y, 3, r,g,b,a);
    thickLineRGBA(renderer, points[4].x, points[4].y, points[5].x, points[5].y, 3, r,g,b,a);
    thickLineRGBA(renderer, points[6].x, points[6].y, points[7].x, points[7].y, 3, r,g,b,a);
}

// تابع رسم سلف

void drawRotatableInductor(SDL_Renderer* renderer, int x, int y, int length, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {

    vector<SDL_Point> local_points;
    const int thickness = 3;
    const int num_loops = 4; // تعداد حلقه‌های سیم‌پیچ
    const int segments_per_loop = 12; // تعداد قطعات برای رسم هر نیم‌دایره (برای نرمی بیشتر)

    const double lead_width_ratio = 0.15; // درصد طول سیم‌های دو سر
    const double coil_width_ratio = 1.0 - (2.0 * lead_width_ratio);

    // تولید سیم ابتدایی
    local_points.push_back({0, 0});
    local_points.push_back({(int)(length * lead_width_ratio), 0});

    // تولید حلقه‌های سیم‌پیچ
    double coil_section_width = length * coil_width_ratio;
    double loop_width = coil_section_width / num_loops;
    double radius = loop_width / 2.0;

    for (int i = 0; i < num_loops; ++i) {
        double loop_center_x = (length * lead_width_ratio) + (i * loop_width) + radius;
        // رسم یک نیم‌دایره به عنوان حلقه
        for (int j = 0; j <= segments_per_loop; ++j) {
            double angle_rad = M_PI - (M_PI * j / segments_per_loop); // زاویه از ۱۸۰ به ۰ درجه
            double point_x = loop_center_x + radius * cos(angle_rad);
            double point_y = 0 - radius * sin(angle_rad); // مختصات y نسبت به خط مرکزی (y=0)
            local_points.push_back({(int)round(point_x), (int)round(point_y)});
        }
    }

    // تولید سیم انتهایی
    local_points.push_back({length, 0});

    // 2. تمام نقاط محلی را چرخانده و به موقعیت اصلی قطعه (x,y) منتقل می‌کنیم
    vector<SDL_Point> world_points;
    double rotation_angle = rotation * M_PI / 2.0;

    for (const auto& pt : local_points) {
        int rotated_x = round(pt.x * cos(rotation_angle) - pt.y * sin(rotation_angle));
        int rotated_y = round(pt.x * sin(rotation_angle) + pt.y * cos(rotation_angle));
        world_points.push_back({x + rotated_x, y + rotated_y});
    }

    // 3. خطوط را بین نقاط نهایی (در مختصات جهانی) رسم می‌کنیم
    for (size_t i = 0; i < world_points.size() - 1; ++i) {
        thickLineRGBA(renderer,
                      world_points[i].x, world_points[i].y,
                      world_points[i+1].x, world_points[i+1].y,
                      thickness, r, g, b, a);
    }
}

// تابع رسم دیود
void drawRotatableDiode(SDL_Renderer* renderer, int x, int y, int length, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    double lead_width = length * 0.4;
    double triangle_width = length * 0.2;
    int triangle_height = 15;

    SDL_Point p1 = {0, 0};
    SDL_Point p2 = {(int)lead_width, 0};
    SDL_Point triangle_v1 = {(int)lead_width, -triangle_height};
    SDL_Point triangle_v2 = {(int)lead_width, triangle_height};
    SDL_Point triangle_v3 = {(int)(lead_width + triangle_width), 0};
    SDL_Point p3 = {(int)(lead_width + triangle_width), -triangle_height};
    SDL_Point p4 = {(int)(lead_width + triangle_width), triangle_height};
    SDL_Point p5 = {(int)(lead_width + triangle_width), 0};
    SDL_Point p6 = {length, 0};

    vector<SDL_Point> all_points = {p1, p2, triangle_v1, triangle_v2, triangle_v3, p3, p4, p5, p6};
    double angle = rotation * M_PI / 2.0;

    for (auto& pt : all_points) {
        int rotated_x = round(pt.x * cos(angle) - pt.y * sin(angle));
        int rotated_y = round(pt.x * sin(angle) + pt.y * cos(angle));
        pt = {x + rotated_x, y + rotated_y};
    }
    Sint16 vx[] = { (Sint16)all_points[2].x, (Sint16)all_points[3].x, (Sint16)all_points[4].x };
    Sint16 vy[] = { (Sint16)all_points[2].y, (Sint16)all_points[3].y, (Sint16)all_points[4].y };

    thickLineRGBA(renderer, all_points[0].x, all_points[0].y, all_points[1].x, all_points[1].y, 3, r,g,b,a);
    filledPolygonRGBA(renderer, vx, vy, 3, r,g,b,a);
    thickLineRGBA(renderer, all_points[5].x, all_points[5].y, all_points[6].x, all_points[6].y, 3, r,g,b,a);
    thickLineRGBA(renderer, all_points[7].x, all_points[7].y, all_points[8].x, all_points[8].y, 3, r,g,b,a);
}

//طراحی ایکون سیم
void drawsimIcon(SDL_Renderer* renderer, int x, int y, int size) {
    // رنگ آیکون (آبی)
    Uint8 r = 0, g = 0, b = 255, a = 255;

    // محاسبه تمام ابعاد به صورت نسبتی از پارامتر 'size'
    // این مقادیر اعشاری به عنوان ضرایب مقیاس‌دهی عمل می‌کنند
    float start_offset_ratio = 0.25f; // فاصله از لبه
    float end_offset_ratio = 0.75f;   // نقطه پایانی
    float radius_ratio = 0.1f;        // شعاع دایره‌ها
    float thickness_ratio = 0.07f;    // ضخامت خط

    // محاسبه مقادیر نهایی بر حسب پیکسل
    int start_pos = (int)(size * start_offset_ratio);
    int end_pos = (int)(size * end_offset_ratio);
    int radius = (int)(size * radius_ratio);
    int line_thickness = (int)(size * thickness_ratio);

    // اطمینان از اینکه شعاع و ضخامت حداقل ۱ پیکسل باشند
    if (radius < 1) radius = 1;
    if (line_thickness < 1) line_thickness = 1;

    // مختصات نهایی با احتساب موقعیت (x, y)
    int start_x = x + start_pos;
    int start_y = y + start_pos;
    int end_x = x + end_pos;
    int end_y = y + end_pos;

    // 1. رسم خطوط ضخیم
    thickLineRGBA(renderer, start_x, start_y, end_x, start_y, line_thickness, r, g, b, a);
    thickLineRGBA(renderer, end_x, start_y, end_x, end_y, line_thickness, r, g, b, a);

    // 2. رسم دایره توپر
    filledCircleRGBA(renderer, start_x, start_y, radius, r, g, b, a);

    // 3. رسم دایره توخالی
    aacircleRGBA(renderer, end_x, end_y, radius, r, g, b, a);
    if (radius > 1) { // برای ضخیم‌تر شدن، اگر امکان داشت
        aacircleRGBA(renderer, end_x, end_y, radius - 1, r, g, b, a);
    }
}
//برای سورسها که فقط پایه ها میچرخن
void drawRotatableSource(SDL_Renderer* renderer, const string& type, int x, int y, int radius, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int lead_length = 20;
    double angle = rotation * M_PI / 2.0;


    SDL_Point lead1_start = {-radius, 0};
    SDL_Point lead1_end = {-radius - lead_length, 0};
    SDL_Point lead2_start = {radius, 0};
    SDL_Point lead2_end = {radius + lead_length, 0};

    vector<SDL_Point> leads = {lead1_start, lead1_end, lead2_start, lead2_end};
    for(auto& pt : leads) {
        int rotated_x = round(pt.x * cos(angle) - pt.y * sin(angle));
        int rotated_y = round(pt.x * sin(angle) + pt.y * cos(angle));
        pt = {x + rotated_x, y + rotated_y};
    }
    thickLineRGBA(renderer, leads[0].x, leads[0].y, leads[1].x, leads[1].y, 3, r,g,b,a);
    thickLineRGBA(renderer, leads[2].x, leads[2].y, leads[3].x, leads[3].y, 3, r,g,b,a);

    filledCircleRGBA(renderer, x, y, radius, 30, 30, 30, 255);
    circleRGBA(renderer, x, y, radius, r,g,b,a);

    vector<SDL_Point> symbol_points;


    if (type == "DCV") {

        symbol_points.push_back({10, -5});
        symbol_points.push_back({10, 5});
        symbol_points.push_back({5, 0});
        symbol_points.push_back({15, 0});

        symbol_points.push_back({-11, -6});
        symbol_points.push_back({-11, 6});
    }
    else if (type == "SINV") {

        for(int i = -12; i <= 12; ++i) {
            symbol_points.push_back({i, (int)(-6 * sin(i * M_PI / 12.0))});
        }
    } else if (type == "DCI") {

        symbol_points.push_back({-10, 0});
        symbol_points.push_back({10, 0});
        symbol_points.push_back({10, 0});
        symbol_points.push_back({4, -5});
        symbol_points.push_back({10, 0});
        symbol_points.push_back({4, 5});
    }


    for(auto& pt : symbol_points) {

        int rotated_x = round(pt.x * cos(angle) - pt.y * sin(angle));
        int rotated_y = round(pt.x * sin(angle) + pt.y * cos(angle));
        pt = {x + rotated_x, y + rotated_y};
    }


    if (type == "DCV" || type == "DCI") {
        for (size_t i = 0; i < symbol_points.size(); i += 2) {
            thickLineRGBA(renderer, symbol_points[i].x, symbol_points[i].y, symbol_points[i+1].x, symbol_points[i+1].y, 2, r, g, b, a);
        }
    } else if (type == "SINV") {
        for (size_t i = 0; i < symbol_points.size() - 1; ++i) {
            thickLineRGBA(renderer, symbol_points[i].x, symbol_points[i].y, symbol_points[i+1].x, symbol_points[i+1].y, 2, r, g, b, a);
        }
    }
}
//رسم خطکشی نمودار سیگنال
void drawPlotGrid(SDL_Renderer* renderer, const SDL_Rect& plotArea) {
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

    const int numDivisions = 10;

    for (int i = 1; i < numDivisions; ++i) {
        int y = plotArea.y + (i * plotArea.h / numDivisions);
        SDL_RenderDrawLine(renderer, plotArea.x, y, plotArea.x + plotArea.w, y);
    }

    for (int i = 1; i < numDivisions; ++i) {
        int x = plotArea.x + (i * plotArea.w / numDivisions);
        SDL_RenderDrawLine(renderer, x, plotArea.y, x, plotArea.y + plotArea.h);
    }
}
// تابع جدید برای رسم آیکون حذف (یک ضربدر قرمز)
void drawDeleteIcon(SDL_Renderer* renderer, int x, int y, int size) {
    Uint8 r = 220, g = 0, b = 0, a = 255; // Red color
    int thickness = size / 6;
    if (thickness < 3) {
        thickness = 3;
    }
    filledCircleRGBA(renderer, x + size / 2, y + size / 2, size / 2, 40, 40, 40, 255);
    circleRGBA(renderer, x + size / 2, y + size / 2, size / 2, 200, 200, 200, 255);

    thickLineRGBA(renderer,
                  x + thickness, y + thickness,
                  x + size - thickness, y + size - thickness,
                  thickness, r, g, b, a);
    thickLineRGBA(renderer,
                  x + size - thickness, y + thickness,
                  x + thickness, y + size - thickness,
                  thickness, r, g, b, a);
}

// تابع کمکی برای محاسبه فاصله یک نقطه تا یک قطعه خط
// این تابع برای تشخیص کلیک روی سیم‌ها و قطعات خطی استفاده می‌شود
double distanceToLineSegment(int px, int py, int x1, int y1, int x2, int y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    if (dx == 0 && dy == 0) { // The segment is a point
        return sqrt(pow(px - x1, 2) + pow(py - y1, 2));
    }
    double t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
    t = max(0.0, min(1.0, t)); // Clamp t to the [0, 1] range
    double closestX = x1 + t * dx;
    double closestY = y1 + t * dy;
    return sqrt(pow(px - closestX, 2) + pow(py - closestY, 2));
}

// تابع جدید برای پیدا کردن یا ساختن گره
Node* findOrCreateNodeAt(int x, int y) {

    for (Node* node : nodes) {
        if (node->x == x && node->y == y) {
            return node;
        }
    }


    string nodeName = "N_" + to_string(x) + "_" + to_string(y);
    Node* newNode = new Node(nodeName, Node_count++, x, y);
    nodes.push_back(newNode);
    return newNode;
}
// تابع جدید برای پردازش سیم ها و تبدیل آنها به مقاومت
void updateCircuitModelFromGUI(Circuit& circuit, const vector<vector<SDL_Point>>& all_wires) {

    Components.erase(
            remove_if(Components.begin(), Components.end(), [](Component* c) {
                if (c->getName().rfind("W_auto_", 0) == 0) {
                    delete c;
                    return true;
                }
                return false;
            }),
            Components.end()
    );


    map<string, SDL_Point> all_junction_points;
    for (const auto& comp : Components) {
        string key1 = to_string(comp->getNode1()->x) + "," + to_string(comp->getNode1()->y);
        all_junction_points[key1] = {comp->getNode1()->x, comp->getNode1()->y};
        string key2 = to_string(comp->getNode2()->x) + "," + to_string(comp->getNode2()->y);
        all_junction_points[key2] = {comp->getNode2()->x, comp->getNode2()->y};
    }
    for (const auto& wire : all_wires) {
        for (const auto& point : wire) {
            string key = to_string(point.x) + "," + to_string(point.y);
            all_junction_points[key] = point;
        }
    }


    int wire_segment_count = 0;
    for (const auto& wire : all_wires) {
        for (size_t i = 0; i < wire.size() - 1; ++i) {
            SDL_Point p1 = wire[i];
            SDL_Point p2 = wire[i+1];


            vector<SDL_Point> points_on_segment;
            points_on_segment.push_back(p1);

            for(const auto& pair : all_junction_points) {
                SDL_Point junction = pair.second;

                if (distanceToLineSegment(junction.x, junction.y, p1.x, p1.y, p2.x, p2.y) < 1.0) {

                    if ( (min(p1.x, p2.x) <= junction.x && junction.x <= max(p1.x, p2.x)) &&
                         (min(p1.y, p2.y) <= junction.y && junction.y <= max(p1.y, p2.y)) )
                    {
                        points_on_segment.push_back(junction);
                    }
                }
            }
            points_on_segment.push_back(p2);


            sort(points_on_segment.begin(), points_on_segment.end(), [&](const SDL_Point& a, const SDL_Point& b) {
                double dist_a = pow(a.x - p1.x, 2) + pow(a.y - p1.y, 2);
                double dist_b = pow(b.x - p1.x, 2) + pow(b.y - p1.y, 2);
                return dist_a < dist_b;
            });


            points_on_segment.erase(unique(points_on_segment.begin(), points_on_segment.end(),
                                           [](const SDL_Point& a, const SDL_Point& b){ return a.x == b.x && a.y == b.y; }), points_on_segment.end());



            for (size_t j = 0; j < points_on_segment.size() - 1; ++j) {
                SDL_Point seg_start = points_on_segment[j];
                SDL_Point seg_end = points_on_segment[j+1];


                if (seg_start.x == seg_end.x && seg_start.y == seg_end.y) continue;

                Node* n_start = findOrCreateNodeAt(seg_start.x, seg_start.y);
                Node* n_end = findOrCreateNodeAt(seg_end.x, seg_end.y);

                string wire_resistor_name = "W_auto_" + to_string(wire_segment_count++);

                circuit.add_resistor(wire_resistor_name, n_start->getName(), n_end->getName(), "0.000000001");
            }
        }
    }
}



void cleanupOrphanedNodes() {
    if (nodes.empty()) return;

    map<Node*, int> connectionCounts;


    for (Component* comp : Components) {
        if (comp->getNode1()) {
            connectionCounts[comp->getNode1()]++;
        }
        if (comp->getNode2()) {
            connectionCounts[comp->getNode2()]++;
        }
    }


    nodes.erase(
            remove_if(nodes.begin(), nodes.end(),
                      [&](Node* node) {

                          if (node == GN) {
                              return false;
                          }

                          if (connectionCounts.find(node) == connectionCounts.end()) {

                              delete node;
                              return true;
                          }
                          return false;
                      }),
            nodes.end()
    );
}

void drawGndIcon(SDL_Renderer* renderer, int x, int y, int size, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int thickness = 3;

    int center_x = x + size / 2;


    thickLineRGBA(renderer, center_x, y, center_x, y + size / 2, thickness, r, g, b, a);


    int line_y = y + size / 2;
    thickLineRGBA(renderer, x, line_y, x + size, line_y, thickness, r, g, b, a);
    line_y += 5;
    thickLineRGBA(renderer, x + size * 0.2, line_y, x + size * 0.8, line_y, thickness, r, g, b, a);
    line_y += 5;
    thickLineRGBA(renderer, x + size * 0.4, line_y, x + size * 0.6, line_y, thickness, r, g, b, a);
}



class AnalysisDialog {
public:
    bool active;
    enum class Result { None, RunTransient, RunAC, Cancel };
    enum class View { Main, Transient, ACSweep };
    View currentView;
    Result result;
    string tran_stop_time;
    string tran_start_time;
    string tran_timestep;

    int ac_sweep_type;
    string ac_start_freq;
    string ac_stop_freq;
    string ac_points;

    int active_field;

    SDL_Rect bg_rect;
    SDL_Rect transient_button_rect;
    SDL_Rect ac_sweep_button_rect;
    vector<SDL_Rect> tran_field_rects;
    vector<SDL_Rect> ac_field_rects;
    vector<SDL_Rect> ac_radio_button_rects;
    SDL_Rect ok_button_rect;
    SDL_Rect cancel_button_rect;
    SDL_Rect back_button_rect;


    AnalysisDialog() : active(false), result(Result::None),currentView(View::Main), active_field(-1) {
        tran_stop_time = "1s";
        tran_start_time = "0";
        tran_timestep = "1u";
        ac_sweep_type = 2;
        ac_start_freq = "1";
        ac_stop_freq = "1k";
        ac_points = "100";
    }

    void setup() {
        active = true;
        currentView = View::Main;
        active_field = -1;
        updateRects();
        SDL_StartTextInput();
    }

    void close() {
        active = false;
        SDL_StopTextInput();
    }

    void updateRects() {
        int dialog_w = 400;
        int dialog_h = 0;

        if (currentView == View::Main) {
            dialog_h = 200;
        } else if (currentView == View::Transient) {
            dialog_h = 250;
        } else if (currentView == View::ACSweep) {
            dialog_h = 320;
        }

        bg_rect = {(SCREEN_WIDTH - dialog_w) / 2, (SCREEN_HEIGHT - dialog_h) / 2, dialog_w, dialog_h};

        if (currentView == View::Main) {
            transient_button_rect = {bg_rect.x + 50, bg_rect.y + 50, dialog_w - 100, 40};
            ac_sweep_button_rect = {bg_rect.x + 50, bg_rect.y + 110, dialog_w - 100, 40};
        } else {
            ok_button_rect = {bg_rect.x + (dialog_w / 2) - 110, bg_rect.y + dialog_h - 45, 100, 30};
            cancel_button_rect = {bg_rect.x + (dialog_w / 2) + 10, bg_rect.y + dialog_h - 45, 100, 30};
            back_button_rect = {bg_rect.x + 15, bg_rect.y + 15, 60, 25};

            if (currentView == View::Transient) {
                tran_field_rects.clear();
                tran_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 60, 180, 30});
                tran_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 100, 180, 30});
                tran_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 140, 180, 30});
            } else if (currentView == View::ACSweep) {
                ac_field_rects.clear();
                ac_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 140, 180, 30});
                ac_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 180, 180, 30});
                ac_field_rects.push_back({bg_rect.x + 200, bg_rect.y + 220, 180, 30});

                ac_radio_button_rects.clear();
                ac_radio_button_rects.push_back({bg_rect.x + 200, bg_rect.y + 80, 15, 15});
                ac_radio_button_rects.push_back({bg_rect.x + 270, bg_rect.y + 80, 15, 15});
                ac_radio_button_rects.push_back({bg_rect.x + 340, bg_rect.y + 80, 15, 15});
            }
        }
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) {
        if (!active) return;

        FilledRect(renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 150);

        FilledRect(renderer, bg_rect.x, bg_rect.y, bg_rect.w, bg_rect.h, 80, 80, 90);
        SDL_SetRenderDrawColor(renderer, 150, 150, 160, 255);
        SDL_RenderDrawRect(renderer, &bg_rect);

        SDL_Color textColor = {255, 255, 255, 255};

        if (currentView == View::Main) {
            drawText(renderer, font, "Select Analysis Type", bg_rect.x + 120, bg_rect.y + 15, textColor);
            FilledRect(renderer, transient_button_rect.x, transient_button_rect.y, transient_button_rect.w, transient_button_rect.h, 70, 90, 130);
            drawText(renderer, font, "Transient Analysis", transient_button_rect.x + 120, transient_button_rect.y + 12, textColor);
            FilledRect(renderer, ac_sweep_button_rect.x, ac_sweep_button_rect.y, ac_sweep_button_rect.w, ac_sweep_button_rect.h, 70, 90, 130);
            drawText(renderer, font, "AC Sweep", ac_sweep_button_rect.x + 150, ac_sweep_button_rect.y + 12, textColor);
        } else {
            FilledRect(renderer, back_button_rect.x, back_button_rect.y, back_button_rect.w, back_button_rect.h, 150, 80, 80);
            drawText(renderer, font, "Back", back_button_rect.x + 15, back_button_rect.y + 5, textColor);

            FilledRect(renderer, ok_button_rect.x, ok_button_rect.y, ok_button_rect.w, ok_button_rect.h, 70, 130, 90);
            drawText(renderer, font, "Run", ok_button_rect.x + 35, ok_button_rect.y + 7, textColor);
            FilledRect(renderer, cancel_button_rect.x, cancel_button_rect.y, cancel_button_rect.w, cancel_button_rect.h, 150, 80, 80);
            drawText(renderer, font, "Cancel", cancel_button_rect.x + 25, cancel_button_rect.y + 7, textColor);

            if (currentView == View::Transient) {
                drawText(renderer, font, "Transient Analysis Settings", bg_rect.x + 100, bg_rect.y + 15, textColor);
                vector<string> labels = {"Stop Time:", "Start Saving Data Time:", "Maximum Timestep:"};
                vector<string*> values = {&tran_stop_time, &tran_start_time, &tran_timestep};

                for (size_t i = 0; i < labels.size(); ++i) {
                    drawText(renderer, font, labels[i], bg_rect.x + 20, bg_rect.y + 65 + i * 40, textColor);
                    FilledRect(renderer, tran_field_rects[i].x, tran_field_rects[i].y, tran_field_rects[i].w, tran_field_rects[i].h, 40, 40, 50);
                    if (active_field == i) {
                        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
                    }
                    SDL_RenderDrawRect(renderer, &tran_field_rects[i]);
                    string text_to_draw = *values[i];
                    if (active_field == i && (SDL_GetTicks() / 500) % 2 == 0) text_to_draw += "|";
                    drawText(renderer, font, text_to_draw, tran_field_rects[i].x + 5, tran_field_rects[i].y + 7, textColor);
                }

            } else if (currentView == View::ACSweep) {
                drawText(renderer, font, "AC Sweep Analysis Settings", bg_rect.x + 100, bg_rect.y + 15, textColor);

                drawText(renderer, font, "Type of Sweep:", bg_rect.x + 20, bg_rect.y + 80, textColor);
                vector<string> radio_labels = {"Octave", "Decade", "Linear"};
                for(int i = 0; i < 3; ++i) {
                    boxRGBA(renderer, ac_radio_button_rects[i].x, ac_radio_button_rects[i].y, ac_radio_button_rects[i].x + ac_radio_button_rects[i].w, ac_radio_button_rects[i].y + ac_radio_button_rects[i].h, 255, 255, 255, 255);
                    if (ac_sweep_type == i) {
                        FilledRect(renderer, ac_radio_button_rects[i].x + 3, ac_radio_button_rects[i].y + 3, ac_radio_button_rects[i].w - 6, ac_radio_button_rects[i].h - 6, 100, 150, 255);
                    }
                    drawText(renderer, font, radio_labels[i], ac_radio_button_rects[i].x + 20, ac_radio_button_rects[i].y, textColor);
                }

                vector<string> labels = {"Start Frequency:", "Stop Frequency:", "Number of points:"};
                vector<string*> values = {&ac_start_freq, &ac_stop_freq, &ac_points};

                for (size_t i = 0; i < labels.size(); ++i) {
                    drawText(renderer, font, labels[i], bg_rect.x + 20, bg_rect.y + 145 + i * 40, textColor);
                    FilledRect(renderer, ac_field_rects[i].x, ac_field_rects[i].y, ac_field_rects[i].w, ac_field_rects[i].h, 40, 40, 50);
                    if (active_field == (int)i + 3) {
                        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
                    }
                    SDL_RenderDrawRect(renderer, &ac_field_rects[i]);
                    string text_to_draw = *values[i];
                    if (active_field == (int)i + 3 && (SDL_GetTicks() / 500) % 2 == 0) text_to_draw += "|";
                    drawText(renderer, font, text_to_draw, ac_field_rects[i].x + 5, ac_field_rects[i].y + 7, textColor);
                }
            }
        }
    }

    void handleEvent(const SDL_Event& event) {
        if (!active) return;
        if (event.type == SDL_KEYDOWN) {
            if ( event.key.keysym.sym == SDLK_ESCAPE )
            {
                result = Result::Cancel;
                close();
            }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_Point mousePt = {event.button.x, event.button.y};
            active_field = -1;

            if (currentView == View::Main) {
                if (SDL_PointInRect(&mousePt, &transient_button_rect)) {
                    currentView = View::Transient;
                    updateRects();
                } else if (SDL_PointInRect(&mousePt, &ac_sweep_button_rect)) {
                    currentView = View::ACSweep;
                    updateRects();
                }
            } else {
                if (SDL_PointInRect(&mousePt, &cancel_button_rect)) {
                    result = Result::Cancel;
                    close();
                }




                else if (SDL_PointInRect(&mousePt, &ok_button_rect)) {

                    if (currentView == View::Transient) {
                        result = Result::RunTransient;

                    } else if (currentView == View::ACSweep) {
                        result = Result::RunAC;

                    }

                    close();
                } else if (SDL_PointInRect(&mousePt, &back_button_rect)) {
                    currentView = View::Main;
                    updateRects();
                }

                if (currentView == View::Transient) {
                    for(size_t i=0; i < tran_field_rects.size(); ++i) {
                        if (SDL_PointInRect(&mousePt, &tran_field_rects[i])) {
                            active_field = i;
                            break;
                        }
                    }
                } else if (currentView == View::ACSweep) {
                    for(int i=0; i<3; ++i) {
                        if (SDL_PointInRect(&mousePt, &ac_radio_button_rects[i])) {
                            ac_sweep_type = i;
                            break;
                        }
                    }
                    for(size_t i=0; i < ac_field_rects.size(); ++i) {
                        if (SDL_PointInRect(&mousePt, &ac_field_rects[i])) {
                            active_field = i + 3;
                            break;
                        }
                    }
                }
            }
        } else if (event.type == SDL_TEXTINPUT && active_field != -1) {
            string* target_string = nullptr;
            if (currentView == View::Transient) {
                if (active_field == 0) target_string = &tran_stop_time;
                else if (active_field == 1) target_string = &tran_start_time;
                else if (active_field == 2) target_string = &tran_timestep;
            } else if (currentView == View::ACSweep) {
                if (active_field == 3) target_string = &ac_start_freq;
                else if (active_field == 4) target_string = &ac_stop_freq;
                else if (active_field == 5) target_string = &ac_points;
            }
            if(target_string) *target_string += event.text.text;

        } else if (event.type == SDL_KEYDOWN && active_field != -1) {
            if (event.key.keysym.sym == SDLK_BACKSPACE) {
                string* target_string = nullptr;
                if (currentView == View::Transient) {
                    if (active_field == 0) target_string = &tran_stop_time;
                    else if (active_field == 1) target_string = &tran_start_time;
                    else if (active_field == 2) target_string = &tran_timestep;
                } else if (currentView == View::ACSweep) {
                    if (active_field == 3) target_string = &ac_start_freq;
                    else if (active_field == 4) target_string = &ac_stop_freq;
                    else if (active_field == 5) target_string = &ac_points;
                }
                if(target_string && !target_string->empty()) target_string->pop_back();
            }
        }
    }
};

bool validateAndUpdateComponent(InputDialog& dialog, Circuit& circuit, SDL_Window* window) {
    Component* comp = dialog.target;
    if (!comp) return false;

    string newName = dialog.values[0];
    string type = comp->getType();


    Component* existingComp = findComponent(newName);
    if (existingComp && existingComp != comp) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Invalid Name",
                                 "A component with this name already exists. Please choose another name.",
                                 window);
        return false;
    }


    double value = 0;
    if (type == "Resistor" || type == "Capacitor" || type == "Inductor") {
        if (type == "Resistor") value = circuit.convertToOhms(dialog.values[1]);
        if (type == "Capacitor") value = circuit.convertToFarad(dialog.values[1]);
        if (type == "Inductor") value = circuit.convertToHenry(dialog.values[1]);

        if (value <= 0) {
            string errorMsg = type + " value must be positive.";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid Value", errorMsg.c_str(), window);
            return false;
        }
    }

    if (type == "SinVoltageSource" && dialog.values.size() == 4) {
        double freq = stod(dialog.values[3]);
        if (freq <= 0) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid Value", "Frequency must be positive.", window);
            return false;
        }
    }


    comp->setName(newName);
    if (type == "Resistor") comp->setValue(circuit.convertToOhms(dialog.values[1]));
    else if (type == "Capacitor") comp->setValue(circuit.convertToFarad(dialog.values[1]));
    else if (type == "Inductor") comp->setValue(circuit.convertToHenry(dialog.values[1]));
    else if (type == "DcVoltageSource") comp->setValue(circuit.convert_to_volts(dialog.values[1]));
    else if (type == "CurrentSource") comp->setValue(circuit.convert_to_amps(dialog.values[1]));
    else if (type == "SinVoltageSource" && dialog.values.size() == 4) {
        double offset = circuit.convert_to_volts(dialog.values[1]);
        double amp = circuit.convert_to_volts(dialog.values[2]);
        double freq = stod(dialog.values[3]);
        static_cast<SinVoltageSource*>(comp)->setSinValues(offset, amp, freq);
    }

    return true;
}
int SDL_main(int argc, char* argv[])
{

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error: SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG))) {
        std::cerr << "Error: SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        return -1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "Error: SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Circuit Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Error: Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Error: Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    //POPUP FOR PLOT:
    //صفحه باز میکنه تا نمودار رو ببینیم
    SDL_Window* plotWindow = nullptr;
    SDL_Renderer* plotRenderer = nullptr;

    AnalysisDialog analysisDialog;
    Circuit Circuit;
    InputDialog InputDialog;
    string componentToPlace = "";
    int componentRotation = 0;
    bool isComponentMenuOpen = false;
    bool cursor_state = true;
    bool panjare_math = false;
    //متغیرای منوی math
    string math_input1 = "V(n1)";
    string math_input2 = "V(n2)";
    int mathhfield = -1;

    SDL_Rect mathInput1Rect, mathInput2Rect;
    SDL_Rect plusButtonRect, minusButtonRect, multiplyButtonRect, divideButtonRect;

    //این برای دابل کرزر هستش
    bool isDoubleCursorActive = false;
    int doubleCursorState = -1;
    double c_t1 = 0, c_v1 = 0, c_t2 = 0, c_v2 = 0;

    bool isDeleteMode = false; //  متغیر جدید برای حالت حذف
    bool isWiringMode = false;              // برای فعال/غیرفعال کردن حالت سیم‌کشی
    vector<SDL_Point> currentWirePoints;    // نقاط سیم در حال رسم
    vector<vector<SDL_Point>> all_wires;    // برای ذخیره تمام سیم‌های کشیده شده

    auto findNodeAt = [&](int x, int y) -> Node* {
        for (Node* node : nodes) {
            if (abs(node->x - x) < FASELE_NOGHAT / 2 && abs(node->y - y) < FASELE_NOGHAT / 2) {
                return node;
            }
        }
        return nullptr;
    };
    //این یه سیگنال نمونه هست، برای راحتی کار این رو اینجا گذاشتیم که بتونیم همیشه یه سیگنال ببینیم و تغییرات رو مشاهده کنیم
    //در آخر که کد کامل شد این تیکه رو پاک میکنیم ولی فعلا اگه کد رو اجرا کنی این تیکه ران میشه و یه سیگنال پالس نشون میده
    vector<double> time_points;
    map<string, vector<double>> temp_sim_results;
    map<string, SDL_Color> trace_colors;
    /*   Node* n_1 = new Node("n1", Node_count++); nodes.push_back(n_1);
       Node* n_2 = new Node("n2", Node_count++); nodes.push_back(n_2);
       Node* n_gnd = new Node("gnd", Node_count++); nodes.push_back(n_gnd);
       GN = n_gnd;
       GN->setVoltage(0.0);
       vector<double> square_wave_values;
       for(int i = 0; i < 200; ++i) {
           square_wave_values.push_back( (i < 100) ? 5.0 : 0.0 );
       }
       WaveformSource* source_v1 = new WaveformSource("V1", n_1, GN, square_wave_values, 0.01);
      Components.push_back(source_v1);
       Resistor* r1 = new Resistor("R1", n_1, n_2, 1000);
       Components.push_back(r1);
       Resistor* r2 = new Resistor("R2", n_2, GN, 1000);
       Components.push_back(r2);

      vector<string> vars_to_print = {"V(n1)", "V(n2)"};
       temp_sim_results = Circuit.runTransientAnalysis(0.01, 1.99, 0.0, vars_to_print, time_points);

       if (!temp_sim_results.empty()) {
           cout << "Simulation complete. " << time_points.size() << " data points generated." << endl;
           trace_colors["V(n1)"] = {100, 255, 100, 255}; // Green
           trace_colors["V(n2)"] = {100, 100, 255, 255}; // Blue
       }
   */


    plotWindow = SDL_CreateWindow(
            "Signal Viewer",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1100,
            600,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );



    if (plotWindow) {
        plotRenderer = SDL_CreateRenderer(plotWindow, -1, SDL_RENDERER_ACCELERATED);
    }
    if (plotRenderer == nullptr) {
        std::cerr << "Error: Plot renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(plotWindow);
        plotWindow = nullptr;
    }

    TTF_Font* font = TTF_OpenFont("font.ttf", 12);

    SDL_Rect plotToolbarRect, plotArea, Auto_zoom, cursor_button_rect, double_cursor_rect, mathButtonRect, mathPanelRect;
    //برای ناحیه کناری سیگنال
    SDL_Rect  plot_sidebar;
    double minTime = 0, maxTime = 0, minVoltage = 0, maxVoltage = 0;

    if (plotWindow && !temp_sim_results.empty()) {

        int plotW, plotH;
        SDL_GetWindowSize(plotWindow, &plotW, &plotH);
        //تول بار برای قسمت نمودار
        const int sidebarWidth = 250;

        plotToolbarRect = {0, 0, plotW, 40};
        plotArea = {60, plotToolbarRect.h + 20, plotW - sidebarWidth - 80, plotH - plotToolbarRect.h - 60};

        plot_sidebar = {plotArea.x + plotArea.w + 20, plotToolbarRect.h + 20, sidebarWidth, plotH - plotToolbarRect.h - 40};

        Auto_zoom = {10, 5, 45, 30};
        cursor_button_rect = {Auto_zoom.x + Auto_zoom.w + 10, 5, 53, 30};
        double_cursor_rect = {cursor_button_rect.x + cursor_button_rect.w + 10, 5, 90, 30};
        mathButtonRect = {double_cursor_rect.x + double_cursor_rect.w + 10, 5, 50, 30};

        if (!time_points.empty()) {
            minTime = time_points.front();
            maxTime = time_points.back();
        }
        if (temp_sim_results.count("V(n1)") > 0 && !temp_sim_results.at("V(n1)").empty()) {
            const auto& voltages = temp_sim_results.at("V(n1)");
            minVoltage = voltages[0];
            maxVoltage = voltages[0];
            for (double v : voltages) {
                if (v < minVoltage) minVoltage = v;
                if (v > maxVoltage) maxVoltage = v;
            }
            double voltageRange = maxVoltage - minVoltage;
            if (voltageRange == 0) { voltageRange = 1.0; }
            double padding = voltageRange * 0.1;
            minVoltage -= padding;
            maxVoltage += padding;
        }
    }
    //اینا برای زوم استفاده شدن، اورجینال ها رو برابر این ویوها قرار میدیم و بعدا با زوم این ویوها رو عوض میکنیم
    //حالا اون بخش AUTO رو هم اینجوری زدیم که هروقت روی اون دکمه زدیم این مقادیر ویو هرچی باشه برمیگردونه به مقادیر اورجینالش
    double view_minT = minTime;
    double view_maxT = maxTime;
    double view_minV = minVoltage;
    double view_maxV = maxVoltage;





    //اینا رو برای قسمت cursor تو plot زدم، اول میایم توی اون پنجره حرکت موس رو track میکنیم
    //منفی 1 گذاشتیم یعنی که موس هنوز تو ویندو پلات نیست هنوز
    int mouse_x_plot = -1;
    int mouse_y_plot = -1;
    int R_count = 1, C_count = 1, L_count = 1, D_count = 1;
    int V_count = 1, I_count = 1;

    bool is_running = true;

    SDL_Event event;
    while (is_running) {


        while (SDL_PollEvent(&event) != 0) {
            if (analysisDialog.active) {
                analysisDialog.handleEvent(event);
                continue;
            }
            if (analysisDialog.result == AnalysisDialog::Result::RunTransient) {
                analysisDialog.result = AnalysisDialog::Result::None;


                double t_stop = Circuit.convertTosecond(analysisDialog.tran_stop_time);
                double t_start = Circuit.convertTosecond(analysisDialog.tran_start_time);
                double t_step = Circuit.convertTosecond(analysisDialog.tran_timestep);


                if (t_step <= 0 || t_stop <= t_start) {

                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Analysis Error", "Invalid transient analysis parameters.", window);
                } else {

                    vector<string> vars_to_print;
                    for (Node* n : nodes) {
                        if (n != GN) {
                            vars_to_print.push_back("V(" + n->getName() + ")");
                        }
                    }
                    for (Component* c : Components) {
                        string type = c->getType();
                        if (type == "DcVoltageSource" || type == "SinVoltageSource" || type == "Inductor") {
                            vars_to_print.push_back("I(" + c->getName() + ")");
                        }
                    }

                    if (vars_to_print.empty()) {

                        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Analysis Warning", "There are no nodes in the circuit to analyze.", window);
                    } else {

                        cout << "\n--- Starting Transient Analysis ---" << endl;
                        Circuit.performTransientAnalysis(t_step, t_stop, t_start, vars_to_print);
                        cout << "--- Transient Analysis Finished ---\n" << endl;
                    }
                }
            }
            if (InputDialog.active) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_TAB) {
                        InputDialog.active_field = (InputDialog.active_field + 1) % InputDialog.values.size();
                    }
                    else if (event.key.keysym.sym == SDLK_BACKSPACE &&
                             !InputDialog.values[InputDialog.active_field].empty()) {
                        InputDialog.values[InputDialog.active_field].pop_back();
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (validateAndUpdateComponent(InputDialog, Circuit, window)) {
                            InputDialog.close();
                            continue;
                        }
                        Component *comp = InputDialog.target;
                        if (comp) {
                            comp->setName(InputDialog.values[0]);
                            string type = comp->getType();
                            if (type == "Resistor") {
                                comp->setValue(Circuit.convertToOhms(InputDialog.values[1]));
                            }
                            else if (type == "Capacitor") {
                                comp->setValue(Circuit.convertToFarad(InputDialog.values[1]));
                            }
                            else if (type == "Inductor") {
                                comp->setValue(Circuit.convertToHenry(InputDialog.values[1]));
                            }
                            else if (type == "DcVoltageSource")
                            {
                                comp->setValue(Circuit.convert_to_volts(InputDialog.values[1]));
                            }
                            else if (type == "CurrentSource")
                            {
                                comp->setValue(Circuit.convert_to_amps(InputDialog.values[1]));
                            }
                            else if (type == "SinVoltageSource" && InputDialog.values.size() == 4) {
                                double offset = Circuit.convert_to_volts(InputDialog.values[1]);
                                double amp = Circuit.convert_to_volts(InputDialog.values[2]);
                                double freq = stod(InputDialog.values[3]);
                                static_cast<SinVoltageSource *>(comp)->setSinValues(offset, amp, freq);
                            }
                        }
                        InputDialog.close();
                    }
                }
                else if (event.type == SDL_TEXTINPUT) {
                    InputDialog.values[InputDialog.active_field] += event.text.text;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Point mousePt = {event.button.x, event.button.y};
                    int mouseX_click, mouseY_click;
                    SDL_GetMouseState(&mouseX_click, &mouseY_click);
                    SDL_Point clickPoint = {mouseX_click, mouseY_click};

                    if (SDL_PointInRect(&mousePt, &InputDialog.ok_button_rect)) {
                        if (validateAndUpdateComponent(InputDialog, Circuit, window)) {
                            InputDialog.close();
                            continue;
                        }
                        Component *comp = InputDialog.target;
                        if (comp) {
                            comp->setName(InputDialog.values[0]);
                            string type = comp->getType();
                            if (type == "Resistor") {
                                comp->setValue(Circuit.convertToOhms(InputDialog.values[1]));
                            }
                            else if (type == "Capacitor") {
                                comp->setValue(Circuit.convertToFarad(InputDialog.values[1]));
                            }
                            else if (type == "Inductor") {
                                comp->setValue(Circuit.convertToHenry(InputDialog.values[1]));
                            }
                            else if (type == "DcVoltageSource")
                                comp->setValue(Circuit.convert_to_volts(InputDialog.values[1]));
                            else if (type == "CurrentSource")
                                comp->setValue(Circuit.convert_to_amps(InputDialog.values[1]));
                            else if (type == "SinVoltageSource" && InputDialog.values.size() == 4) {
                                double offset = Circuit.convert_to_volts(InputDialog.values[1]);
                                double amp = Circuit.convert_to_volts(InputDialog.values[2]);
                                double freq = stod(InputDialog.values[3]);
                                static_cast<SinVoltageSource *>(comp)->setSinValues(offset, amp, freq);
                            }
                        }
                        InputDialog.close();
                    } else if (SDL_PointInRect(&mousePt, &InputDialog.cancel_button_rect)) {
                        if (InputDialog.target) {
                            string type = InputDialog.target->getType();
                            string name = InputDialog.target->getName();
                            if (type == "Resistor") Circuit.deleteResistor(name);
                            else if (type == "Capacitor") Circuit.deleteCapacitor(name);
                            else if (type == "Inductor") Circuit.deleteInductor(name);
                            else if (type == "Diode") Circuit.deleteDiode(name);
                        }
                        InputDialog.close();
                    }
                    else {
                        for (size_t i = 0; i < InputDialog.field_rects.size(); ++i) {
                            if (SDL_PointInRect(&mousePt, &InputDialog.field_rects[i])) {
                                InputDialog.active_field = i;
                                break;
                            }
                        }
                    }

                    if (panjare_math) {
                        if (SDL_PointInRect(&clickPoint, &mathInput1Rect)) {
                            mathhfield = 0;
                        }
                        else if (SDL_PointInRect(&clickPoint, &mathInput2Rect)) {
                            mathhfield = 1;
                        }
                    }
                }
            }
            else {
                if (event.type == SDL_QUIT) {
                    is_running = false;
                }
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        is_running = false;
                    }
                }
                //برای زوم
                if (event.type == SDL_MOUSEWHEEL) {
                    if (plotWindow && event.wheel.windowID == SDL_GetWindowID(plotWindow)) {
                        double zoomFactor = (event.wheel.y > 0) ? 1.15 : (1.0 / 1.15);

                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);

                        double timeCenter =
                                view_minT + ((mouseX - plotArea.x) / (double) plotArea.w) * (view_maxT - view_minT);
                        zoom(view_minT, view_maxT, zoomFactor, timeCenter);

                        double voltageCenter =
                                view_maxV - ((mouseY - plotArea.y) / (double) plotArea.h) * (view_maxV - view_minV);
                        zoom(view_minV, view_maxV, zoomFactor, voltageCenter);
                    }
                }
                if (event.type == SDL_KEYDOWN) {
                    if (plotWindow && SDL_GetKeyboardFocus() == plotWindow) {
                        double zoom_factor = 1.25;
                        double time_center = view_minT + (view_maxT - view_minT) / 2.0;
                        double voltageCenter = view_minV + (view_maxV - view_minV) / 2.0;

                        switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                zoom(view_minV, view_maxV, 1 / zoom_factor, voltageCenter);
                                break;
                            case SDLK_DOWN:
                                zoom(view_minV, view_maxV, zoom_factor, voltageCenter);
                                break;
                            case SDLK_RIGHT:
                                zoom(view_minT, view_maxT, 1 / zoom_factor, time_center);
                                break;
                            case SDLK_LEFT:
                                zoom(view_minT, view_maxT, zoom_factor, time_center);
                                break;
                        }
                    }
                }

                //برای حرکت موس در پنجره
                if (event.type == SDL_MOUSEMOTION) {
                    if (plotWindow && event.motion.windowID == SDL_GetWindowID(plotWindow)) {
                        mouse_x_plot = event.motion.x;
                        mouse_y_plot = event.motion.y;
                    }
                }

                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX_click, mouseY_click;
                    SDL_GetMouseState(&mouseX_click, &mouseY_click);
                    SDL_Point clickPoint = {mouseX_click, mouseY_click};

                    int delete_icon_x =  (40 + 25) * 6; // یک موقعیت مثال
                    SDL_Rect deleteIconRect = {delete_icon_x, 5, 40, 40};

                    if (SDL_PointInRect(&clickPoint, &deleteIconRect)) {
                        isDeleteMode = !isDeleteMode; // وضعیت حالت حذف را تغییر بده
                        if (isDeleteMode) {
                            // هنگام ورود به حالت حذف، حالت های دیگر را غیرفعال کن
                            isWiringMode = false;
                            componentToPlace = "";
                            isComponentMenuOpen = false;
                            currentWirePoints.clear();
                        }
                    }

                    // محدوده آیکون کامپوننت را دوباره اینجا تعریف می‌کنیم تا کلیک را تشخیص دهیم
                    // این مقادیر باید با مقادیر بخش رسم هماهنگ باشند
                    int temp_x = 20 + (40 + 25) * 4; // محاسبه موقت موقعیت x آیکون کامپوننت
                    int comp_h = 40;
                    int comp_w = comp_h * 0.7;
                    int comp_pin_len = comp_w / 5;
                    int comp_total_width = comp_w + (2 * comp_pin_len);
                    SDL_Rect componentClickRect = {temp_x, 5, comp_total_width, comp_h};

                    int sim_icon_x = (40 + 25) * 5;
                    SDL_Rect simClickRect = {sim_icon_x, 5, 60, 40};

                    if (SDL_PointInRect(&clickPoint, &simClickRect)) {
                        isWiringMode = !isWiringMode;
                        componentToPlace = "";
                        currentWirePoints.clear();

                    }
                    if (SDL_PointInRect(&clickPoint, &componentClickRect)) {
                        isComponentMenuOpen = !isComponentMenuOpen;
                        isWiringMode = false; // از حالت سیم کشی خارج شو
                        currentWirePoints.clear();
                    }
                    int play_icon_x = 20 + (40+25)*3;
                    int play_icon_height = 40;
                    int play_icon_width = play_icon_height * 0.866;
                    SDL_Rect playFileRect = {play_icon_x, 5, play_icon_width, play_icon_height};

                    if (SDL_PointInRect(&clickPoint, &playFileRect)) {

                        analysisDialog.setup();
                    }
                    if (plotWindow && event.button.windowID == SDL_GetWindowID(plotWindow)) {
                        if (event.button.clicks == 2) {
                            cout << "Double click detected in plot window!" << endl;
                        }
                        else{
                            int m_x, m_y;
                            SDL_GetMouseState(&m_x, &m_y);
                            SDL_Point mousePoint = {m_x, m_y};
                            if (SDL_PointInRect(&mousePoint, &Auto_zoom)) {
                                view_minT = minTime;
                                view_maxT = maxTime;
                                view_minV = minVoltage;
                                view_maxV = maxVoltage;
                            }
                            //برای دکمه cursor زدم
                            if (SDL_PointInRect(&mousePoint, &cursor_button_rect)) {
                                cursor_state = !cursor_state;
                            }
                            else if (SDL_PointInRect(&mousePoint, &double_cursor_rect)) {
                                isDoubleCursorActive = !isDoubleCursorActive;
                                cursor_state = false;
                                doubleCursorState = -1;
                            }
                                //برای عملیت ریاضی
                            else if (SDL_PointInRect(&mousePoint, &mathButtonRect)) {
                                panjare_math = !panjare_math;
                                //برای خوندن نوشته تو قسمتایmath سیگنال
                                if (panjare_math) {
                                    SDL_StartTextInput();
                                }
                                else {
                                    SDL_StopTextInput();
                                }
                            }
                            else if (isDoubleCursorActive && SDL_PointInRect(&mousePoint, &plotArea)) {
                                double t_at_click = view_minT + ((mousePoint.x - plotArea.x) / (double)plotArea.w) * (view_maxT - view_minT);
                                double voltageAtClick = view_maxV - ((mousePoint.y - plotArea.y) / (double)plotArea.h) * (view_maxV - view_minV);

                                string closestSignalName = "";
                                int closest_index = -1;
                                double minVoltageDist = -1;

                                for (map<string, vector<double>>::const_iterator it = temp_sim_results.begin(); it != temp_sim_results.end(); ++it) {
                                    const string& name = it->first;
                                    const vector<double>& signalVector = it->second;

                                    int i1 = -1, i2 = -1;
                                    if (!time_points.empty()) {
                                        for (int i = 0; i < time_points.size() - 1; ++i) {
                                            if (t_at_click >= time_points[i] && t_at_click <= time_points[i + 1]) {
                                                i1 = i;
                                                i2 = i + 1;
                                                break;
                                            }
                                        }
                                    }
                                    if (i1 != -1) {
                                        double v1 = signalVector[i1], v2 = signalVector[i2];
                                        double t1 = time_points[i1], t2 = time_points[i2];
                                        double timeFraction = (t2 - t1 == 0) ? 0 : (t_at_click - t1) / (t2 - t1);
                                        double interpolatedVoltage = v1 + timeFraction * (v2 - v1);
                                        double dist = abs(interpolatedVoltage - voltageAtClick);

                                        if (minVoltageDist == -1 || dist < minVoltageDist) {
                                            minVoltageDist = dist;
                                            closestSignalName = name;
                                            closest_index = (abs(t_at_click - t1) < abs(t_at_click - t2)) ? i1 : i2;
                                        }
                                    }
                                }

                                if (closest_index != -1) {
                                    if (doubleCursorState != 0) {
                                        c_t1 = time_points[closest_index];
                                        c_v1 = temp_sim_results.at(closestSignalName)[closest_index];
                                        doubleCursorState = 0;
                                    }
                                    else {
                                        c_t2 = time_points[closest_index];
                                        c_v2 = temp_sim_results.at(closestSignalName)[closest_index];
                                        doubleCursorState = 1;
                                    }
                                }
                            }
                        }
                    }
                    if (isWiringMode) {
                        if (mouseY_click > 50) {
                            int snapped_x = round((float) mouseX_click / FASELE_NOGHAT) * FASELE_NOGHAT;
                            int snapped_y = round((float) mouseY_click / FASELE_NOGHAT) * FASELE_NOGHAT;

                            if (currentWirePoints.empty()) {
                                currentWirePoints.push_back({snapped_x, snapped_y});
                            }
                            else {

                                SDL_Point last_point = currentWirePoints.back();

                                if (last_point.x != snapped_x) {
                                    currentWirePoints.push_back({snapped_x, last_point.y});
                                }
                                currentWirePoints.push_back({snapped_x, snapped_y});
                                all_wires.push_back(currentWirePoints);
                                currentWirePoints.clear();
                                updateCircuitModelFromGUI(Circuit, all_wires);
                            }
                        }
                    }
                    else if (isDeleteMode) {
                        bool itemDeleted = false;
                        //شعاع کلیک
                        const int R = 10;
                        if (GN != nullptr && GN->x != -1) {

                            double dist_to_gnd = sqrt(pow(mouseX_click - GN->x, 2) + pow(mouseY_click - (GN->y + 15), 2));
                            if (dist_to_gnd < 20) {

                                auto it = find_if(nodes.begin(), nodes.end(), [&](Node* node) {
                                    return node == GN;
                                });
                                if (it != nodes.end()) {
                                    delete *it;
                                    nodes.erase(it);
                                }
                                GN = nullptr;
                                cout << "Ground node deleted." << endl;
                                itemDeleted = true;
                            }
                        }
                        for (auto it = Components.begin(); it != Components.end(); ) {
                            Component* comp = *it;
                            Node* n1 = comp->getNode1();
                            Node* n2 = comp->getNode2();

                            if (n1->x == -1 || n2->x == -1) {
                                ++it;
                                continue;
                            }
                            bool isClicked = false;
                            string type = comp->getType();

                            if (type == "DcVoltageSource" || type == "SinVoltageSource" || type == "CurrentSource") {
                                int center_x = n1->x + (n2->x - n1->x) / 2;
                                int center_y = n1->y + (n2->y - n1->y) / 2;
                                double dist = sqrt(pow(mouseX_click - center_x, 2) + pow(mouseY_click - center_y, 2));
                                if (dist < 20 + R) {
                                    isClicked = true;
                                }
                            } else {
                                if (distanceToLineSegment(mouseX_click, mouseY_click, n1->x, n1->y, n2->x, n2->y) < R) {
                                    isClicked = true;
                                }
                            }

                            if (isClicked) {
                                // قطعه را از کلاس Circuit حذف کن
                                // نکته: توابع deleteResistor و غیره فقط از وکتور حذف می‌کنند و حافظه را آزاد می‌کنند.
                                // ما اینجا از findComponent برای اطمینان استفاده می‌کنیم.
                                if (findComponent(comp->getName())) {
                                    if(type == "Resistor") Circuit.deleteResistor(comp->getName());
                                    else if(type == "Capacitor") Circuit.deleteCapacitor(comp->getName());
                                    else if(type == "Inductor") Circuit.deleteInductor(comp->getName());
                                    else if(type == "Diode") Circuit.deleteDiode(comp->getName());
                                    else if(type == "DcVoltageSource" || type == "SinVoltageSource" || type == "WaveformSource") {
                                        Circuit.deleteVoltageSource(comp->getName());
                                    }
                                    else if(type == "CurrentSource") {
                                        Circuit.deleteCurrentSource(comp->getName());
                                    }
                                }
                                itemDeleted = true;
                                break;
                            }
                            ++it;
                        }

                        if (!itemDeleted) {
                            for (auto it_wire = all_wires.begin(); it_wire != all_wires.end(); ++it_wire) {
                                for (size_t i = 0; i < it_wire->size() - 1; ++i) {
                                    SDL_Point p1 = (*it_wire)[i];
                                    SDL_Point p2 = (*it_wire)[i+1];
                                    if (distanceToLineSegment(mouseX_click, mouseY_click, p1.x, p1.y, p2.x, p2.y) < R) {
                                        all_wires.erase(it_wire);
                                        itemDeleted = true;
                                        break;
                                    }
                                }
                                if (itemDeleted) {
                                    updateCircuitModelFromGUI(Circuit, all_wires);
                                    cleanupOrphanedNodes();
                                    break;
                                }
                            }
                        }

                    }
                    else {
                        //  اگر در حالت جایگذاری یک قطعه هستیم، آن را روی صفحه قرار بده
                        if (!componentToPlace.empty()) {
                            if (mouseY_click > 50 && !(isComponentMenuOpen && mouseX_click < 200)) {
                                if (componentToPlace == "GND") {

                                    if (GN != nullptr) {

                                        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                                                 "Error",
                                                                 "A ground node already exists. Delete the current one to place a new one.",
                                                                 window);
                                    } else {

                                        int snapped_x = round((float)mouseX_click / FASELE_NOGHAT) * FASELE_NOGHAT;
                                        int snapped_y = round((float)mouseY_click / FASELE_NOGHAT) * FASELE_NOGHAT;


                                        Node* gndNode = findOrCreateNodeAt(snapped_x, snapped_y);


                                        Circuit.add_ground_node(gndNode->getName());

                                    }
                                    componentToPlace = "";
                                }

                                else {
                                    int snapped_x = round((float) mouseX_click / FASELE_NOGHAT) * FASELE_NOGHAT;
                                    int snapped_y = round((float) mouseY_click / FASELE_NOGHAT) * FASELE_NOGHAT;


                                    int len = (componentToPlace == "DCV" || componentToPlace == "SINV" || componentToPlace == "DCI")
                                              ? 4 * FASELE_NOGHAT
                                              : 6 * FASELE_NOGHAT;
                                    int half_len = len / 2;
                                    double angle = componentRotation * M_PI / 2.0;
                                    int dx = round(half_len * cos(angle));
                                    int dy = round(half_len * sin(angle));

                                    dx = round((float)dx / FASELE_NOGHAT) * FASELE_NOGHAT;
                                    dy = round((float)dy / FASELE_NOGHAT) * FASELE_NOGHAT;

                                    int start_x = snapped_x - dx;
                                    int start_y = snapped_y - dy;
                                    int end_x = snapped_x + dx;
                                    int end_y = snapped_y + dy;


                                    Node* n1 = findOrCreateNodeAt(start_x, start_y);
                                    Node* n2 = findOrCreateNodeAt(end_x, end_y);

                                    string name;
                                    if (componentToPlace == "Resistor") {
                                        name = "R" + to_string(R_count++);
                                        Circuit.add_resistor(name, n1->getName(), n2->getName(), "1k");
                                    }
                                    else if (componentToPlace == "Capacitor") {
                                        name = "C" + to_string(C_count++);
                                        Circuit.add_capacitor(name, n1->getName(), n2->getName(), "1uF");
                                    }
                                    else if (componentToPlace == "Inductor") {
                                        name = "L" + to_string(L_count++);
                                        Circuit.add_inductor(name, n1->getName(), n2->getName(), "1mH");
                                    }
                                    else if (componentToPlace == "Diode") {
                                        name = "D" + to_string(D_count++);
                                        Circuit.add_diode(name, n1->getName(), n2->getName(), "D");
                                    }
                                    else if (componentToPlace == "DCV") {
                                        name = "V" + to_string(V_count++);
                                        Circuit.add_dc_voltage_source(name, n1->getName(), n2->getName(), "5V");
                                    }
                                    else if (componentToPlace == "SINV") {
                                        name = "V" + to_string(V_count++);
                                        Circuit.add_sin_voltage_source(name, n1->getName(), n2->getName(), "0", "5", "1k");
                                    }
                                    else if (componentToPlace == "DCI") {
                                        name = "I" + to_string(I_count++);
                                        Circuit.add_current_source(name, n1->getName(), n2->getName(), "1A");
                                    }

                                    Component* newComp = findComponent(name);
                                    if (newComp) {

                                        newComp->getNode1()->x = start_x;
                                        newComp->getNode1()->y = start_y;
                                        newComp->getNode2()->x = end_x;
                                        newComp->getNode2()->y = end_y;

                                        InputDialog.setup(newComp, mouseX_click, mouseY_click);
                                        updateCircuitModelFromGUI(Circuit, all_wires);
                                    }
                                    componentToPlace = "";
                                }
                            }
                        }
                        else {
                            //برسی کلیلک ها
                            if (isComponentMenuOpen) {
                                int item_x = 10, current_y = 60, item_h = 70;
                                if (clickPoint.x > item_x && clickPoint.x < item_x + 180) {
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "Resistor";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "Capacitor";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "Inductor";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "Diode";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h + 20;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "DCV";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h + 30;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "SINV";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h + 30;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "DCI";
                                        isComponentMenuOpen = false;
                                    }
                                    current_y += item_h + 10;
                                    if (clickPoint.y > current_y && clickPoint.y < current_y + item_h) {
                                        componentToPlace = "GND";
                                        isComponentMenuOpen = false;
                                    }
                                }
                            }
                        }
                    }
                }
                if (event.type == SDL_KEYDOWN) {
                    if (!componentToPlace.empty()) {
                        //چرخش قطعه
                        if (event.key.keysym.sym == SDLK_r) {
                            componentRotation = (componentRotation + 1) % 4;
                        }
                    }
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        if (isWiringMode) {
                            isWiringMode = false;
                            currentWirePoints.clear();

                        }
                        if (!componentToPlace.empty()) {
                            componentToPlace = "";
                        }
                        if (isDeleteMode) {
                            isDeleteMode = false;
                        }
                    }
                    if (event.key.keysym.sym == SDLK_u) {
                        cout << "\n--- Circuit Netlist ---" << endl;
                        cout << "Total Components: " << Components.size() << endl;
                        cout << "Total Nodes: " << nodes.size() << endl;
                        if (GN) {
                            cout << "Ground Node: " << GN->getName() << endl;
                        } else {
                            cout << "Warning: No Ground node is set!" << endl;
                        }
                        cout << "-------------------------" << endl;

                        if (Components.empty()) {
                            cout << "No components in the circuit." << endl;
                        } else {
                            for (Component* comp : Components) {
                                cout << " - Name: " << left << setw(15) << comp->getName()
                                     << "Type: " << left << setw(18) << comp->getType()
                                     << "Nodes: " << left << setw(12) << comp->getNode1()->getName()
                                     << " -> " << left << setw(12) << comp->getNode2()->getName()
                                     << "Value: " << comp->getvalue() << endl;
                            }
                        }
                        cout << "--- End of Netlist ---\n" << endl;
                    }


                    if (event.key.keysym.sym == SDLK_a) {
                        componentToPlace = "Resistor";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_s) {
                        componentToPlace = "Capacitor";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_d) {
                        componentToPlace = "Inductor";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_f) {
                        componentToPlace = "Diode";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_g) {
                        componentToPlace = "DCV";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_h) {
                        componentToPlace = "SINV";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_j) {
                        componentToPlace = "DCI";
                        isComponentMenuOpen = false;
                    }

                    if (event.key.keysym.sym == SDLK_k) {
                        componentToPlace = "GND";
                        isComponentMenuOpen = false;
                    }



                }
            }
            if (panjare_math) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Point mousePoint = {event.button.x, event.button.y};
                    int mouseX_click, mouseY_click;
                    SDL_GetMouseState(&mouseX_click, &mouseY_click);
                    SDL_Point clickPoint = {mouseX_click, mouseY_click};
                    if (SDL_PointInRect(&clickPoint, &mathInput1Rect)) {
                        mathhfield = 0;
                    }
                    else if (SDL_PointInRect(&clickPoint, &mathInput2Rect)) {
                        mathhfield = 1;
                    }
                        //برای عملیات ریاضی هست
                    else if (SDL_PointInRect(&clickPoint, &plusButtonRect)) {
                        if (temp_sim_results.count(math_input1) && temp_sim_results.count(math_input2)) {
                            vector<double>& sig1 = temp_sim_results.at(math_input1);
                            vector<double>& sig2 = temp_sim_results.at(math_input2);
                            if (sig1.size() == sig2.size()) {
                                vector<double> newSignal;
                                for (size_t i = 0; i < sig1.size(); ++i) newSignal.push_back(sig1[i] + sig2[i]);

                                string newName = "(" + math_input1 + ")+(" + math_input2 + ")";
                                temp_sim_results[newName] = newSignal;
                                trace_colors[newName] = {255, 100, 255, 255}; // Magenta

                                panjare_math = false;
                                SDL_StopTextInput();
                            }
                        }
                    }
                    else if (SDL_PointInRect(&clickPoint, &minusButtonRect)) {
                        if (temp_sim_results.count(math_input1) && temp_sim_results.count(math_input2)) {

                            vector<double>& sig1 = temp_sim_results.at(math_input1);
                            vector<double>& sig2 = temp_sim_results.at(math_input2);

                            if (sig1.size() == sig2.size()) {
                                vector<double> newSignal;
                                for (size_t i = 0; i < sig1.size(); ++i) {
                                    newSignal.push_back(sig1[i] - sig2[i]);
                                }

                                string newName = "(" + math_input1 + ")-(" + math_input2 + ")";
                                temp_sim_results[newName] = newSignal;
                                trace_colors[newName] = {255, 255 ,100, 255};

                                panjare_math = false;
                                SDL_StopTextInput();
                            }
                        }
                    }
                    else if (SDL_PointInRect(&clickPoint, &multiplyButtonRect)) {
                        if (temp_sim_results.count(math_input1) && temp_sim_results.count(math_input2)) {

                            vector<double>& sig1 = temp_sim_results.at(math_input1);
                            vector<double>& sig2 = temp_sim_results.at(math_input2);

                            if (sig1.size() == sig2.size()) {
                                vector<double> newSignal;
                                for (size_t i = 0; i < sig1.size(); ++i) {
                                    newSignal.push_back(sig1[i] * sig2[i]);
                                }
                                string newName = "(" + math_input1 + ") * (" + math_input2 + ")";
                                temp_sim_results[newName] = newSignal;
                                trace_colors[newName] = {0, 128 ,128, 255};
                                panjare_math = false;
                                SDL_StopTextInput();
                            }
                        }
                    }
                    else if (SDL_PointInRect(&clickPoint, &divideButtonRect)) {
                        if (temp_sim_results.count(math_input1) && temp_sim_results.count(math_input2)) {

                            vector<double>& sig1 = temp_sim_results.at(math_input1);
                            vector<double>& sig2 = temp_sim_results.at(math_input2);

                            if (sig1.size() == sig2.size()) {
                                vector<double> newSignal;
                                for (size_t i = 0; i < sig1.size(); ++i) {
                                    newSignal.push_back(sig1[i] / sig2[i]);
                                }

                                string newName = "(" + math_input1 + ") / (" + math_input2 + ")";
                                temp_sim_results[newName] = newSignal;
                                trace_colors[newName] = {128, 0 ,0, 255};
                                panjare_math = false;
                                SDL_StopTextInput();
                            }
                        }
                    }
                    else {
                        mathhfield = -1;
                    }

                }
                if (event.type == SDL_TEXTINPUT) {
                    if (mathhfield == 0) {
                        math_input1 += event.text.text;
                    } else if (mathhfield == 1) {
                        math_input2 += event.text.text;
                    }
                }
                if (event.type == SDL_KEYDOWN && mathhfield != -1) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        if (mathhfield == 0 && !math_input1.empty()) {
                            math_input1.pop_back();
                        } else if (mathhfield == 1 && !math_input2.empty()) {
                            math_input2.pop_back();
                        }
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        draw_grid(renderer);
        FilledRect(renderer, 0 , 0, 1600, 50, 255, 255, 255);


        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = {mouseX, mouseY};

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        draw_grid(renderer);
        FilledRect(renderer, 0 , 0, 1600, 50, 255, 255, 255);

        // رسم جایگذاری شذه
        SDL_Color componentTextColor = {200, 200, 200, 255}; // A light gray color

// رسم قطعات جایگذاری شده
        for (Component* comp : Components) {
            if (comp->getName().rfind("W_auto_", 0) == 0) {
                continue;
            }
            Node* n1 = comp->getNode1();
            Node* n2 = comp->getNode2();

            if (n1->x != -1 && n2->x != -1) {
                int dx = n2->x - n1->x;
                int dy = n2->y - n1->y;

                int center_x = n1->x + dx / 2;
                int center_y = n1->y + dy / 2;

                int length = round(sqrt(dx * dx + dy * dy));

                double angle_rad = atan2(dy, dx);
                int rotation_idx = round(angle_rad * 2.0 / M_PI);
                if (rotation_idx < 0) rotation_idx += 4;
                rotation_idx = (rotation_idx + 4) % 4;

                Uint8 r=255, g=255, b=0, a=255;

                string compType = comp->getType();
                int start_x = n1->x;
                int start_y = n1->y;

                if (compType == "Resistor") {

                    drawRotatableResistor(renderer, start_x, start_y, length, rotation_idx, r,g,b,a);
                }
                else if (compType == "Capacitor") {
                    drawRotatableCapacitor(renderer, start_x, start_y, length, rotation_idx, r,g,b,a);
                }
                else if (compType == "Inductor") {
                    drawRotatableInductor(renderer, start_x, start_y, length, rotation_idx, r,g,b,a);
                }
                else if (compType == "Diode") {
                    drawRotatableDiode(renderer, start_x, start_y, length, rotation_idx, r,g,b,a);
                }
                else if (compType == "DcVoltageSource") {
                    drawRotatableSource(renderer, "DCV", center_x, center_y, 20, rotation_idx, r,g,b,a);
                }
                else if (compType == "SinVoltageSource") {
                    drawRotatableSource(renderer, "SINV", center_x, center_y, 20, rotation_idx, r,g,b,a);
                }
                else if (compType == "CurrentSource") {
                    drawRotatableSource(renderer, "DCI", center_x, center_y, 20, rotation_idx, r,g,b,a);
                }


                if (font) {
                    string nameStr = comp->getName();
                    string valueStr = Circuit.formatValue(comp);

                    int text_w_name, text_h_name, text_w_value, text_h_value;
                    TTF_SizeText(font, nameStr.c_str(), &text_w_name, &text_h_name);
                    TTF_SizeText(font, valueStr.c_str(), &text_w_value, &text_h_value);

                    int text_x_name, text_y_name, text_x_value, text_y_value;
                    int offset = 20;

                    if (rotation_idx == 0 || rotation_idx == 2) {

                        text_x_name = center_x - text_w_name / 2;
                        text_y_name = center_y - offset - text_h_name;
                        text_x_value = center_x - text_w_value / 2;
                        text_y_value = center_y + offset;


                    }
                    else { // Vertical

                        int gap = 5;
                        int total_text_height = text_h_name + gap + text_h_value;
                        int start_y = center_y - total_text_height / 2;

                        text_x_name = center_x + offset;
                        text_y_name = start_y;

                        text_x_value = center_x + offset;
                        text_y_value = start_y + text_h_name + gap;



                    }

                    drawText(renderer, font, nameStr, text_x_name, text_y_name, componentTextColor);
                    if (!valueStr.empty()) {
                        drawText(renderer, font, valueStr, text_x_value, text_y_value, componentTextColor);
                    }
                }
            }

        }
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);

        for (const auto& wire : all_wires) {
            if (wire.size() >= 2) {
                for (size_t i = 0; i < wire.size() - 1; ++i) {
                    thickLineRGBA(renderer, wire[i].x, wire[i].y, wire[i+1].x, wire[i+1].y, 2, 0, 150, 255, 255);
                }
            }
        }

        map<string, bool> drawn_junctions;


        for (Component* comp : Components) {


            Node* n1 = comp->getNode1();
            Node* n2 = comp->getNode2();
            if (n1 && n1->x != -1) {
                string key = to_string(n1->x) + "," + to_string(n1->y);
                if (!drawn_junctions[key]) {
                    filledCircleRGBA(renderer, n1->x, n1->y, 4, 0, 150, 255, 255);
                    circleRGBA(renderer, n1->x, n1->y, 4, 255, 255, 255, 255);
                    drawn_junctions[key] = true;
                }
            }
            if (n2 && n2->x != -1) {
                string key = to_string(n2->x) + "," + to_string(n2->y);
                if (!drawn_junctions[key]) {
                    filledCircleRGBA(renderer, n2->x, n2->y, 4, 0, 150, 255, 255);
                    circleRGBA(renderer, n2->x, n2->y, 4, 255, 255, 255, 255);
                    drawn_junctions[key] = true;
                }
            }
        }


        for (const auto& wire : all_wires) {
            for (const auto& point : wire) {
                string key = to_string(point.x) + "," + to_string(point.y);
                if (!drawn_junctions[key]) {

                    int connection_count = 0;
                    for (const auto& other_wire : all_wires) {
                        for(const auto& other_point : other_wire) {
                            if (other_point.x == point.x && other_point.y == point.y) {
                                connection_count++;
                                break;
                            }
                        }
                    }
                    if (connection_count > 1) {
                        filledCircleRGBA(renderer, point.x, point.y, 4, 0, 150, 255, 255);
                        circleRGBA(renderer, point.x, point.y, 4, 255, 255, 255, 255);
                        drawn_junctions[key] = true;
                    }
                }
            }
        }
        if (GN != nullptr && GN->x != -1) {

            drawGndIcon(renderer, GN->x - 15, GN->y, 30, 180, 255, 180, 255);
        }
        if (isWiringMode && !currentWirePoints.empty()) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
            SDL_Point start_point = currentWirePoints.back();

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            int snapped_x = round((float)mouseX / FASELE_NOGHAT) * FASELE_NOGHAT;
            int snapped_y = round((float)mouseY / FASELE_NOGHAT) * FASELE_NOGHAT;

            SDL_RenderDrawLine(renderer, start_point.x, start_point.y, snapped_x, start_point.y);
            SDL_RenderDrawLine(renderer, snapped_x, start_point.y, snapped_x, snapped_y);
        }
        if (!componentToPlace.empty()) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            int snapped_x = round((float)mouseX / FASELE_NOGHAT) * FASELE_NOGHAT;
            int snapped_y = round((float)mouseY / FASELE_NOGHAT) * FASELE_NOGHAT;

            Uint8 r=255, g=255, b=255, a=150;

            int len = (componentToPlace == "DCV" || componentToPlace == "SINV" || componentToPlace == "DCI")
                      ? 4 * FASELE_NOGHAT
                      : 6 * FASELE_NOGHAT;
            int radius = 20;

            int half_len = len / 2;
            double angle = componentRotation * M_PI / 2.0;

            int dx = round(half_len * cos(angle));
            int dy = round(half_len * sin(angle));

            dx = round((float)dx / FASELE_NOGHAT) * FASELE_NOGHAT;
            dy = round((float)dy / FASELE_NOGHAT) * FASELE_NOGHAT;

            int start_x = snapped_x - dx;
            int start_y = snapped_y - dy;

            int center_x = snapped_x;
            int center_y = snapped_y;


            if (componentToPlace == "Resistor") {
                drawRotatableResistor(renderer, start_x, start_y, len, componentRotation, r,g,b,a);
            } else if (componentToPlace == "Capacitor") {
                drawRotatableCapacitor(renderer, start_x, start_y, len, componentRotation, r,g,b,a);
            } else if (componentToPlace == "Inductor") {
                drawRotatableInductor(renderer, start_x, start_y, len, componentRotation, r,g,b,a);
            } else if (componentToPlace == "Diode") {
                drawRotatableDiode(renderer, start_x, start_y, len, componentRotation, r,g,b,a);
            } else if (componentToPlace == "DCV") {
                drawRotatableSource(renderer, "DCV", center_x, center_y, radius, componentRotation, r,g,b,a);
            } else if (componentToPlace == "SINV") {
                drawRotatableSource(renderer, "SINV", center_x, center_y, radius, componentRotation, r,g,b,a);
            } else if (componentToPlace == "DCI") {
                drawRotatableSource(renderer, "DCI", center_x, center_y, radius, componentRotation, r,g,b,a);
            }
            else if (componentToPlace == "GND") {

                if (mouseY > 50) {
                    int snapped_x = round((float) mouseX / FASELE_NOGHAT) * FASELE_NOGHAT;
                    int snapped_y = round((float) mouseY / FASELE_NOGHAT) * FASELE_NOGHAT;

                    drawGndIcon(renderer, snapped_x - 15, snapped_y, 30, 255, 255, 255, 150);
                }
            }
        }

        int menu_current_x = 20;
        int padding = 25;
        int icon_y = 5;
        int icon_height = 40;

        int new_file_size = icon_height;
        SDL_Rect newFileRect = {menu_current_x, icon_y, new_file_size, icon_height};
        if (SDL_PointInRect(&mousePoint, &newFileRect)) {
            FilledRect(renderer, newFileRect.x-10, newFileRect.y, newFileRect.w+20, newFileRect.h, 220, 220, 220);
        }
        drawNewFileIcon(renderer, menu_current_x, icon_y, new_file_size);
        menu_current_x += new_file_size + padding;

        int open_file_width = (int)(icon_height / 0.90);
        SDL_Rect openFileRect = {menu_current_x, icon_y, open_file_width, icon_height};
        if (SDL_PointInRect(&mousePoint, &openFileRect)) {
            FilledRect(renderer, openFileRect.x-10, openFileRect.y, openFileRect.w+20, openFileRect.h, 220, 220, 220);
        }
        drawOpenFileIcon(renderer, menu_current_x, icon_y, open_file_width);
        menu_current_x += open_file_width + padding;

        int save_icon_size = icon_height;
        SDL_Rect saveFileRect = {menu_current_x, icon_y, save_icon_size, icon_height};
        if (SDL_PointInRect(&mousePoint, &saveFileRect)) {
            FilledRect(renderer, saveFileRect.x-10, saveFileRect.y, saveFileRect.w+20, saveFileRect.h, 220, 220, 220);
        }
        drawSaveIcon(renderer, menu_current_x, icon_y, save_icon_size);
        menu_current_x += save_icon_size + padding;

        int play_icon_height = icon_height;
        int play_icon_width = play_icon_height * 0.866;
        SDL_Rect playFileRect = {menu_current_x, icon_y, play_icon_width, play_icon_height};
        if (SDL_PointInRect(&mousePoint, &playFileRect)) {
            FilledRect(renderer, playFileRect.x-10, playFileRect.y, playFileRect.w+20, playFileRect.h, 220, 220, 220);
        }
        drawPlayIcon(renderer, menu_current_x, icon_y, play_icon_height);
        menu_current_x += play_icon_width + padding;

        int component_h = icon_height;
        int component_w = component_h * 0.7;
        int component_pin_len = component_w / 5;
        int component_total_width = component_w + (2 * component_pin_len);
        SDL_Rect componentFileRect = {menu_current_x, icon_y, component_total_width, component_h};
        if (SDL_PointInRect(&mousePoint, &componentFileRect)) {
            FilledRect(renderer, componentFileRect.x-10, componentFileRect.y, componentFileRect.w+20, componentFileRect.h, 220, 220, 220);
        }
        drawComponentIcon(renderer, menu_current_x + component_pin_len, icon_y, component_w, component_h);
        if (isComponentMenuOpen) {
            SDL_Rect componentMenuRect = {0, 50, 200, SCREEN_HEIGHT - 50};
            FilledRect(renderer, componentMenuRect.x, componentMenuRect.y, componentMenuRect.w, componentMenuRect.h, 100, 100, 100);

            int item_x = 20;
            int current_y = 70;
            int item_height = 50;
            int vertical_padding = 30;
            Uint8 icon_color[] = {255, 255, 255, 255};
            Uint8 hover_color[] = {130, 130, 130, 255};

            SDL_Rect resistorRect = {item_x-10  , current_y - 10, 120, item_height+20};
            if (SDL_PointInRect(&mousePoint, &resistorRect)) {
                FilledRect(renderer, resistorRect.x, resistorRect.y, resistorRect.w+20, resistorRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawResistorIcon(renderer, item_x, current_y + item_height / 2, 100, 15, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect capacitorRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &capacitorRect)) {
                FilledRect(renderer, capacitorRect.x, capacitorRect.y, capacitorRect.w, capacitorRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawCapacitorIcon(renderer, item_x, current_y, 120, 30, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect inductorRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &inductorRect)) {
                FilledRect(renderer, inductorRect.x, inductorRect.y, inductorRect.w, inductorRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawInductorIcon_Rounded(renderer, item_x, current_y + item_height / 2, 120, 30, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect diodeRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &diodeRect)) {
                FilledRect(renderer, diodeRect.x, diodeRect.y, diodeRect.w, diodeRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawDiodeIcon(renderer, item_x, current_y, 120, 30, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect dcVolRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &dcVolRect)) {
                FilledRect(renderer, dcVolRect.x, dcVolRect.y, dcVolRect.w, dcVolRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawDCVoltageSourceIcon(renderer, item_x + 60, current_y + item_height / 2, 20, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect sinVolRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &sinVolRect)) {
                FilledRect(renderer, sinVolRect.x, sinVolRect.y, sinVolRect.w, sinVolRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawSineVoltageSourceIcon(renderer, item_x + 60, current_y + item_height / 2, 20, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect currentSrcRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &currentSrcRect)) {
                FilledRect(renderer, currentSrcRect.x, currentSrcRect.y, currentSrcRect.w, currentSrcRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }
            drawCurrentSourceIcon(renderer, item_x + 60, current_y + item_height / 2, 20, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);
            current_y += item_height + vertical_padding;

            SDL_Rect gndRect = {item_x - 10, current_y - 10, 180, item_height};
            if (SDL_PointInRect(&mousePoint, &gndRect)) {
                FilledRect(renderer, gndRect.x, gndRect.y, gndRect.w, gndRect.h, hover_color[0], hover_color[1], hover_color[2]);
            }

            drawGndIcon(renderer, item_x + 45, current_y, 30, icon_color[0], icon_color[1], icon_color[2], icon_color[3]);

        }
        menu_current_x += component_w + padding;

        int sim_icon_height = icon_height;
        int sim_icon_width = sim_icon_height * 0.866;
        SDL_Rect simFileRect = {menu_current_x, icon_y, sim_icon_width, sim_icon_height};
        if (SDL_PointInRect(&mousePoint, &simFileRect)) {
            FilledRect(renderer, simFileRect.x, simFileRect.y, simFileRect.w+30, simFileRect.h, 220, 220, 220);
        }
        drawsimIcon(renderer, menu_current_x, icon_y-10,60);
        menu_current_x += sim_icon_width + padding;

        SDL_Rect deleteIconRect = {menu_current_x, icon_y, icon_height, icon_height};
        if (isDeleteMode) {
            FilledRect(renderer, deleteIconRect.x-5, deleteIconRect.y-2, deleteIconRect.w+10, deleteIconRect.h+4, 255, 150, 150);
        }
        else if (SDL_PointInRect(&mousePoint, &deleteIconRect)) {
            FilledRect(renderer, deleteIconRect.x-5, deleteIconRect.y-2, deleteIconRect.w+10, deleteIconRect.h+4, 220, 220, 220);
        }
        drawDeleteIcon(renderer, menu_current_x, icon_y, icon_height);

        if (plotWindow) {
            int plotW, plotH;
            SDL_GetWindowSize(plotWindow, &plotW, &plotH);
            SDL_SetRenderDrawColor(plotRenderer, 0, 0, 0, 255);
            SDL_RenderClear(plotRenderer);

            FilledRect(plotRenderer, plotToolbarRect.x, plotToolbarRect.y, plotToolbarRect.w, plotToolbarRect.h, 50, 50, 50);

            FilledRect(plotRenderer, Auto_zoom.x, Auto_zoom.y, Auto_zoom.w, Auto_zoom.h, 80, 80, 80);
            drawText(plotRenderer, font, "Auto", Auto_zoom.x + 8, Auto_zoom.y + 7, {255, 255, 255, 255});

            Uint8 cursor_bg_r = cursor_state ? 120 : 80;
            FilledRect(plotRenderer, cursor_button_rect.x, cursor_button_rect.y, cursor_button_rect.w, cursor_button_rect.h,
                       cursor_bg_r, 80, 80);
            drawText(plotRenderer, font, "Cursor", cursor_button_rect.x + 8, cursor_button_rect.y + 7,
                     {255, 255, 255, 255});
            //رسم دابل کرزر باتن!


            FilledRect(plotRenderer, double_cursor_rect.x, double_cursor_rect.y, double_cursor_rect.w, double_cursor_rect.h, 80, 80, 80);
            drawText(plotRenderer, font, "Double Cursor", double_cursor_rect.x + 8, double_cursor_rect.y + 7, {255, 255, 255, 255});
            //
            //برای math میزنیم
            Uint8 math_bg_r = panjare_math ? 120 : 80; // Highlight if active
            FilledRect(plotRenderer, mathButtonRect.x, mathButtonRect.y, mathButtonRect.w, mathButtonRect.h, math_bg_r, 80, 80);
            drawText(plotRenderer, font, "Math", mathButtonRect.x + 10, mathButtonRect.y + 7, {255, 255, 255, 255});
            drawPlotGrid(plotRenderer, plotArea);


            drawAxes(plotRenderer, font, plotArea, view_minT, view_maxT, view_minV, view_maxV);

            //بعد از این که Math تکمیل شد یه ساید بار زدم که صرفا میاد سیگنال های موجود با رنگ ها رو نشون میده
            FilledRect(plotRenderer, plot_sidebar.x, plot_sidebar.y, plot_sidebar.w, plot_sidebar.h, 0, 0, 0, 255);

            drawText(plotRenderer, font, "Traces:", plot_sidebar.x + 10, plot_sidebar.y + 5, {255, 255, 255, 255});

            int sidebar_item_position = plot_sidebar.y + 30;
            for (auto const& pair : trace_colors) {
                const string& name = pair.first;
                const SDL_Color& color = pair.second;

                FilledRect(plotRenderer, plot_sidebar.x + 10, sidebar_item_position, 15, 15, color.r, color.g, color.b, color.a);

                drawText(plotRenderer, font, name, plot_sidebar.x + 35, sidebar_item_position + 2, color);

                sidebar_item_position += 20;
            }

            if (temp_sim_results.count("V(n1)")) {
                SDL_SetRenderDrawColor(plotRenderer, 100, 255, 100, 255); // V(n1) is ALWAYS green
                drawSignal(plotRenderer, time_points, temp_sim_results.at("V(n1)"), plotArea, view_minT, view_maxT, view_minV, view_maxV);
            }


            if (temp_sim_results.count("V(n2)")) {
                SDL_SetRenderDrawColor(plotRenderer, 100, 100, 255, 255); // V(n2) is ALWAYS blue
                drawSignal(plotRenderer, time_points, temp_sim_results.at("V(n2)"), plotArea, view_minT, view_maxT, view_minV, view_maxV);
            }
            //برای کشیدن فانکشن های ریاضی با رنگ خودشون
            for (map<string, vector<double>>::const_iterator it = temp_sim_results.begin(); it != temp_sim_results.end(); ++it) {
                const string& name = it->first;
                const vector<double>& signalVector = it->second;

                SDL_Color signalColor = trace_colors[name];

                SDL_SetRenderDrawColor(plotRenderer, signalColor.r, signalColor.g, signalColor.b, signalColor.a);

                drawSignal(plotRenderer, time_points, signalVector, plotArea, view_minT, view_maxT, view_minV, view_maxV);
            }

            if(cursor_state) {
                if (mouse_x_plot >= plotArea.x && mouse_x_plot <= plotArea.x + plotArea.w &&
                    mouse_y_plot >= plotArea.y && mouse_y_plot <= plotArea.y + plotArea.h)
                {
                    double timeAtCursor = view_minT + ((mouse_x_plot - plotArea.x) / (double)plotArea.w) * (view_maxT - view_minT);
                    double voltageAtMouse = view_maxV - ((mouse_y_plot - plotArea.y) / (double)plotArea.h) * (view_maxV - view_minV);

                    string closestSignalName = "";
                    double closestVoltage = 0;
                    double minVoltageDist = -1;

                    for (map<string, vector<double>>::const_iterator it = temp_sim_results.begin(); it != temp_sim_results.end(); ++it) {
                        const string& name = it->first;
                        const vector<double>& signalVector = it->second;

                        int i1 = -1, i2 = -1;
                        if (!time_points.empty()) {
                            for (int i = 0; i < time_points.size() - 1; ++i) {
                                if (timeAtCursor >= time_points[i] && timeAtCursor <= time_points[i + 1]) {
                                    i1 = i; i2 = i + 1; break;
                                }
                            }
                        }
                        if (i1 != -1) {
                            double t1 = time_points[i1], v1 = signalVector[i1];
                            double t2 = time_points[i2], v2 = signalVector[i2];
                            double timeFraction = (t2 - t1 == 0) ? 0 : (timeAtCursor - t1) / (t2 - t1);
                            double interpolatedVoltage = v1 + timeFraction * (v2 - v1);
                            double dist = abs(interpolatedVoltage - voltageAtMouse);
                            if (minVoltageDist == -1 || dist < minVoltageDist) {
                                minVoltageDist = dist;
                                closestSignalName = name;
                                closestVoltage = interpolatedVoltage;
                            }
                        }
                    }

                    if (!closestSignalName.empty()) {
                        stringstream ss;
                        ss << closestSignalName << " Time: " << fixed << setprecision(4) << timeAtCursor << "s";
                        int plotW, plotH;
                        SDL_GetWindowSize(plotWindow, &plotW, &plotH);
                        drawText(plotRenderer, font, ss.str(), plotW - 200, plotH - 70, {255, 255, 0, 255});
                        ss.str("");
                        ss << "Voltage: " << fixed << setprecision(4) << closestVoltage << "V";
                        drawText(plotRenderer, font, ss.str(), plotW - 200, plotH - 55, {255, 255, 0, 255});
                    }
                }
            }
            if (isDoubleCursorActive && doubleCursorState >= 0) {
                const double timeRange = view_maxT - view_minT;
                const double voltageRange = view_maxV - view_minV;
                SDL_Color c1_color = {255, 100, 100, 255};
                SDL_Color c2_color = {100, 100, 255, 255};
                //رسم دبل کرزرها
                int c1_screenX = plotArea.x + ((c_t1 - view_minT) / timeRange) * plotArea.w;
                int c1_screenY = plotArea.y + plotArea.h - ((c_v1 - view_minV) / voltageRange) * plotArea.h;
                vlineRGBA(plotRenderer, c1_screenX, plotArea.y, plotArea.y + plotArea.h, 255, 100, 100, 100);
                filledCircleRGBA(plotRenderer, c1_screenX, c1_screenY, 5, c1_color.r, c1_color.g, c1_color.b, 255);

                if (doubleCursorState == 1) {
                    int c2_screenX = plotArea.x + ((c_t2 - view_minT) / timeRange) * plotArea.w;
                    int c2_screenY = plotArea.y + plotArea.h - ((c_v2 - view_minV) / voltageRange) * plotArea.h;
                    vlineRGBA(plotRenderer, c2_screenX, plotArea.y, plotArea.y + plotArea.h, 100, 100, 255, 100);
                    filledCircleRGBA(plotRenderer, c2_screenX, c2_screenY, 5, c2_color.r, c2_color.g, c2_color.b, 255);

                    double dt = abs(c_t2 - c_t1);
                    double dv = abs(c_v2 - c_v1);

                    int boxW = 140, boxH = 115;
                    int boxX = plotW - boxW - 10;
                    int boxY = plotToolbarRect.h + 5;

                    FilledRect(plotRenderer, boxX, boxY, boxW, boxH, 40, 40, 40, 200); // Dark, semi-transparent background
                    boxRGBA(plotRenderer, boxX, boxY, boxX + boxW, boxY + boxH, 100, 100, 100, 255); // Gray border

                    int labelX = boxX + 5;
                    int valueX_RightEdge = boxX + boxW - 5;
                    int currentY = boxY + 5;
                    int textW, textH;
                    stringstream ss;
                    SDL_Color white = {255, 255, 255, 255};

                    drawText(plotRenderer, font, "T1:", labelX, currentY, c1_color);
                    ss.str(""); ss << fixed << setprecision(4) << c_t1 << "s";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH); // Get width of text
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, c1_color);
                    currentY += 15;

                    drawText(plotRenderer, font, "V1:", labelX, currentY, c1_color);
                    ss.str(""); ss << fixed << setprecision(4) << c_v1 << "V";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH);
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, c1_color);
                    currentY += 20;

                    drawText(plotRenderer, font, "T2:", labelX, currentY, c2_color);
                    ss.str(""); ss << fixed << setprecision(4) << c_t2 << "s";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH);
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, c2_color);
                    currentY += 15;

                    drawText(plotRenderer, font, "V2:", labelX, currentY, c2_color);
                    ss.str(""); ss << fixed << setprecision(4) << c_v2 << "V";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH);
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, c2_color);
                    currentY += 20;

                    drawText(plotRenderer, font, "dT:", labelX, currentY, white);
                    ss.str(""); ss << fixed << setprecision(4) << dt << "s";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH);
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, white);
                    currentY += 15;

                    drawText(plotRenderer, font, "dV:", labelX, currentY, white);
                    ss.str(""); ss << fixed << setprecision(4) << dv << "V";
                    TTF_SizeText(font, ss.str().c_str(), &textW, &textH);
                    drawText(plotRenderer, font, ss.str(), valueX_RightEdge - textW, currentY, white);
                }
            }
            if (font) {
                const int numDivisions = 10;

                double voltageScalePerDiv = (view_maxV - view_minV) / numDivisions;
                double timeScalePerDiv = (view_maxT - view_minT) / numDivisions;

                stringstream ss;
                ss << fixed << setprecision(2) << voltageScalePerDiv << " V/div";
                string vScaleText = ss.str();

                ss.str("");
                ss << fixed << setprecision(3) << timeScalePerDiv * 1000 << " ms/div"; // Display time in milliseconds
                string tScaleText = ss.str();

                int plotW, plotH;
                SDL_GetWindowSize(plotWindow, &plotW, &plotH);

                SDL_Color scaleColor = {200, 200, 200, 255};
                drawText(plotRenderer, font, vScaleText, plotW - 140, plotH - 40, scaleColor);
                drawText(plotRenderer, font, tScaleText, plotW - 140, plotH - 25, scaleColor);
            }
            if (panjare_math) {
                mathPanelRect = {50, 50, 300, 200};

                FilledRect(plotRenderer, mathPanelRect.x, mathPanelRect.y, mathPanelRect.w, mathPanelRect.h, 40, 0, 0, 255);

                boxRGBA(plotRenderer, mathPanelRect.x, mathPanelRect.y, mathPanelRect.x + mathPanelRect.w, mathPanelRect.y + mathPanelRect.h, 150, 150, 150, 255);

                drawText(plotRenderer, font, "Add Math Trace", mathPanelRect.x + 10, mathPanelRect.y + 10, {110, 0, 0, 255});
                SDL_Color textColor = {220, 220, 220, 255};
                int input_w = 200;
                int input_h = 25;
                int label_x = mathPanelRect.x + 15;
                int input_x = mathPanelRect.x + 85;
                //مستطیل های اینپوت برای نوشتن تو قسمت math
                drawText(plotRenderer, font, "Signal 1:", label_x, mathPanelRect.y + 50, textColor);
                mathInput1Rect = {input_x, mathPanelRect.y + 45, input_w, input_h};
                FilledRect(plotRenderer, mathInput1Rect.x, mathInput1Rect.y, mathInput1Rect.w, mathInput1Rect.h, 20, 20, 20, 255);

                if (mathhfield == 0) boxRGBA(plotRenderer, mathInput1Rect.x, mathInput1Rect.y, mathInput1Rect.x + input_w, mathInput1Rect.y + input_h, 100, 150, 255, 255);
                drawText(plotRenderer, font, math_input1, mathInput1Rect.x + 5, mathInput1Rect.y + 5, textColor);

                drawText(plotRenderer, font, "Signal 2:", label_x, mathPanelRect.y + 90, textColor);
                mathInput2Rect = {input_x, mathPanelRect.y + 85, input_w, input_h};
                FilledRect(plotRenderer, mathInput2Rect.x, mathInput2Rect.y, mathInput2Rect.w, mathInput2Rect.h, 20, 20, 20, 255);

                if (mathhfield == 1) boxRGBA(plotRenderer, mathInput2Rect.x, mathInput2Rect.y, mathInput2Rect.x + input_w, mathInput2Rect.y + input_h, 100, 150, 255, 255);
                drawText(plotRenderer, font, math_input2, mathInput2Rect.x + 5, mathInput2Rect.y + 5, textColor);

                int btn_y = mathPanelRect.y + 135;
                int btn_size = 40;
                int btn_padding = 15;
                plusButtonRect = {input_x, btn_y, btn_size, btn_size};
                minusButtonRect = {input_x + btn_size + btn_padding, btn_y, btn_size, btn_size};
                multiplyButtonRect = {input_x + 2*(btn_size + btn_padding), btn_y, btn_size, btn_size};
                divideButtonRect = {input_x + 3*(btn_size + btn_padding), btn_y, btn_size, btn_size};

                FilledRect(plotRenderer, plusButtonRect.x, plusButtonRect.y, plusButtonRect.w, plusButtonRect.h, 80, 80, 80, 255);
                drawText(plotRenderer, font, "+", plusButtonRect.x + 15, plusButtonRect.y + 12, textColor);

                FilledRect(plotRenderer, minusButtonRect.x, minusButtonRect.y, minusButtonRect.w, minusButtonRect.h, 80, 80, 80, 255);
                drawText(plotRenderer, font, "-", minusButtonRect.x + 18, minusButtonRect.y + 12, textColor);

                FilledRect(plotRenderer, multiplyButtonRect.x, multiplyButtonRect.y, multiplyButtonRect.w, multiplyButtonRect.h, 80, 80, 80, 255);
                drawText(plotRenderer, font, "*", multiplyButtonRect.x + 17, multiplyButtonRect.y + 12, textColor);

                FilledRect(plotRenderer, divideButtonRect.x, divideButtonRect.y, divideButtonRect.w, divideButtonRect.h, 80, 80, 80, 255);
                drawText(plotRenderer, font, "/", divideButtonRect.x + 18, divideButtonRect.y + 12, textColor);
            }

        }
        if (isDeleteMode) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            // یک ضربدر قرمز کوچک در محل نشانگر ماوس رسم کن
            if (mouseY > 50) { // فقط در فضای کاری، نه روی نوار ابزار
                drawDeleteIcon(renderer, mouseX - 10, mouseY - 10, 20);
            }
        }

        if (plotWindow) {
            SDL_RenderPresent(plotRenderer);
        }


        if (InputDialog.active) {

            FilledRect(renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 150);


            FilledRect(renderer, InputDialog.bg_rect.x, InputDialog.bg_rect.y, InputDialog.bg_rect.w, InputDialog.bg_rect.h, 80, 80, 90);

            SDL_SetRenderDrawColor(renderer, 150, 150, 160, 255);
            SDL_RenderDrawRect(renderer, &InputDialog.bg_rect);


            SDL_Color textColor = {255, 255, 255, 255};
            for (size_t i = 0; i < InputDialog.labels.size(); ++i) {

                drawText(renderer, font, InputDialog.labels[i], InputDialog.bg_rect.x + 20, InputDialog.bg_rect.y + 55 + i * 40, textColor);


                FilledRect(renderer, InputDialog.field_rects[i].x, InputDialog.field_rects[i].y, InputDialog.field_rects[i].w, InputDialog.field_rects[i].h, 40, 40, 50);

                if (InputDialog.active_field == i) {
                    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
                }
                SDL_RenderDrawRect(renderer, &InputDialog.field_rects[i]);


                string text_to_draw = InputDialog.values[i];

                if (InputDialog.active_field == i && (SDL_GetTicks() / 500) % 2 == 0) {
                    text_to_draw += "|";
                }
                drawText(renderer, font, text_to_draw, InputDialog.field_rects[i].x + 5, InputDialog.field_rects[i].y + 7, textColor);
            }


            FilledRect(renderer, InputDialog.ok_button_rect.x, InputDialog.ok_button_rect.y, InputDialog.ok_button_rect.w, InputDialog.ok_button_rect.h, 70, 130, 90);
            drawText(renderer, font, "OK", InputDialog.ok_button_rect.x + 40, InputDialog.ok_button_rect.y + 7, textColor);

            FilledRect(renderer, InputDialog.cancel_button_rect.x, InputDialog.cancel_button_rect.y, InputDialog.cancel_button_rect.w, InputDialog.cancel_button_rect.h, 150, 80, 80);
            drawText(renderer, font, "Cancel", InputDialog.cancel_button_rect.x + 25, InputDialog.cancel_button_rect.y + 7, textColor);
        }
        if (analysisDialog.active) {
            analysisDialog.draw(renderer, font);
        }


        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    for (Node* node : nodes) { delete node; }
    nodes.clear();
    for (Component* comp : Components) { delete comp; }
    Components.clear();

    cout << "Simulator GUI successfully shut down." << std::endl;

    string line;
    while(getline(cin,line))
    {
        if(line.empty())
        {
            continue;
        }
        stringstream SS(line);
        string input1;
        SS>>input1;
        if(input1 == "end")
        {
            cout<<"end of app"<<endl;
            break;

        }
        else if (input1 == ".print") {
            string analysis_type;
            SS >> analysis_type;

            if (analysis_type == "TRAN") {
                string tstep_str, tstop_str, tstart_str;
                if (!(SS >> tstep_str >> tstop_str >> tstart_str)) {
                    cout << "Error: Syntax Error" << endl;
                    continue;
                }

                vector<string> variables;
                string var;
                while (SS >> var)
                {
                    variables.push_back(var);
                }

                if (variables.empty()) {
                    cout << "Error: Syntax Error" << endl;
                    continue;
                }

                Circuit.performTransientAnalysis(Circuit.convertTosecond(tstep_str), Circuit.convertTosecond(tstop_str), Circuit.convertTosecond(tstart_str), variables);

            } else if (analysis_type == "DC") {
                string sourcename, start_str, end_str, increment_str, variable;
                if (!(SS >> sourcename >> start_str >> end_str >> increment_str >> variable)) {
                    cout << "Syntax Error: .print DC sourcename start end increment variable" << endl;
                    continue;
                }

                bool error_found = false;
                if (variable.front() == 'V' && variable.back() == ')') {
                    string node_name = variable.substr(2, variable.length() - 3);
                    if (findnode(node_name) == nullptr) {
                        cout << "node " << node_name << " not found in circuit" << endl;
                        error_found = true;
                    }
                } else if (variable.front() == 'I' && variable.back() == ')') {
                    string comp_name = variable.substr(2, variable.length() - 3);
                    if (findComponent(comp_name) == nullptr) {
                        cout << "Component " << comp_name << " not found in circuit" << endl;
                        error_found = true;
                    }
                } else
                {
                    cout << "Error: Invalid print variable format." << endl;
                    error_found = true;
                }

                if (!error_found) {
                    Circuit.performDCSweepAnalysis(sourcename, stod(start_str), stod(end_str), stod(increment_str), variable);
                }
            }
        }
            /*
            else if(input1 ==  "add")
            {
                string typeName;
                SS>>typeName;
                const regex add_dc_voltage_pattern(R"(VoltageSource\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+))");
                const regex add_sin_voltage_pattern(R"(V(\w+)\s+(\S+)\s+(\S+)\s+SIN\s*\(\s*(\S+)\s+(\S+)\s+(\S+)\s*\))");
                const regex add_current_pattern(R"(CurrentSource\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+))");
                string full_command_after_add = typeName;
                string remaining_of_line;
                getline(SS, remaining_of_line);
                stringstream GG (remaining_of_line);

                full_command_after_add += remaining_of_line;
                full_command_after_add.erase(0, full_command_after_add.find_first_not_of(" \t"));
                smatch matches;
                string type = typeName.substr(0,1);

                if( Circuit.checkGnd(typeName))
                {
                    if(typeName=="GND")
                    {
                        string nodeName_for_ground;
                        GG >> nodeName_for_ground;
                        if (GG.fail() || nodeName_for_ground.empty()) {
                            cout << "Error: Syntax error for GND." << endl;
                            continue;
                        }
                        Circuit.add_ground_node(nodeName_for_ground);
                    }
                    else
                    {
                        cout << "Error: Element "<<typeName<<" not found in library" << endl;
                        continue;
                    }
                }
                else if(type == "R")
                {
                    string node1, node2, value;
                    GG>>node1>>node2>>value;
                    if(node2 == node1)
                    {
                        cout << "Error: Duplicate node name detected" << endl;
                        continue;
                    }
                    Circuit.add_resistor(typeName,node1,node2, value);

                }
                else if(type == "C")
                {
                    string node1, node2, value;
                    GG>>node1>>node2>>value;
                    if(node2 == node1)
                    {
                        cout << "Error: Duplicate node name detected" << endl;
                        continue;
                    }
                    Circuit.add_capacitor (typeName,node1,node2, value);

                }
                else if(type == "L")
                {
                    string node1, node2, value;
                    GG>>node1>>node2>>value;
                    if(node2 == node1)
                    {
                        cout << "Error: Duplicate node name detected" << endl;
                        continue;
                    }
                    Circuit.add_inductor(typeName,node1,node2, value);

                }
                else if(type == "D")
                {
                    string node1, node2, model;
                    GG>>node1>>node2>>model;
                    if(node2 == node1)
                    {
                        cout << "Error: Duplicate node name detected" << endl;
                        continue;
                    }
                    Circuit.add_diode(typeName, node1, node2, model);
                }
                else if (regex_match(full_command_after_add, matches, add_dc_voltage_pattern)) {
                    string parsed_typeName = matches[1].str();
                    string node1 = matches[2].str();
                    string node2 = matches[3].str();
                    string value = matches[4].str();
                    Circuit.add_dc_voltage_source(parsed_typeName, node1, node2, value);

                }
                else if (regex_match(full_command_after_add, matches, add_sin_voltage_pattern)) {
                    string parsed_typeName_part = matches[1].str();
                    string node1 = matches[2].str();
                    string node2 = matches[3].str();
                    string voffset_str = matches[4].str();
                    string vamplitude_str = matches[5].str();
                    string frequency_str = matches[6].str();
                    string full_typeName = "V" + parsed_typeName_part;
                    Circuit.add_sin_voltage_source(full_typeName, node1, node2, voffset_str, vamplitude_str, frequency_str);

                }
                else if (regex_match(full_command_after_add, matches, add_current_pattern)) {
                    string parsed_typeName = matches[1].str();
                    string node1 = matches[2].str();
                    string node2 = matches[3].str();
                    string value = matches[4].str();
                    Circuit.add_current_source(parsed_typeName, node1, node2, value);
                }
                else
                {
                    cout << "Error: Syntax error" << endl;
                }

            }
             */
        else if (input1 == "delete")
        {
            string typeName;
            SS >> typeName;
            string type = typeName.substr(0, 1);

            if( Circuit.checkGnd(typeName))
            {
                if(typeName=="GND")
                {
                    string nodeName_for_ground;
                    SS >> nodeName_for_ground;
                    if (SS.fail())
                    {
                        cout << "Error: Syntax error." << endl;
                        continue;
                    }
                    Circuit.delete_ground_node(nodeName_for_ground);
                }
                else
                {
                    cout << "Error: Element "<<typeName<<" not found in library" << endl;
                    continue;
                }
            }
            else if (type == "N")
            {


                Circuit.deleteNode(typeName);
            }
            else if (type == "R")
            {


                Circuit.deleteResistor(typeName);
            }
            else if (type == "C")
            {


                Circuit.deleteCapacitor(typeName);
            }

            else if (type == "L")
            {


                Circuit.deleteInductor(typeName);
            }
            else if (type == "D")
            {
                Circuit.deleteDiode(typeName);
            }
            else
            {
                cout << "Error: Syntax error" << endl;
            }
        }
        else if (input1 == ".nodes")
        {
            Circuit.show_nodes();
        }
        else if (input1 == ".list")
        {
            string input2;
            SS>>input2;
            if(input2.empty())
            {
                Circuit.show_all();
            }
            else
            {
                Circuit. showbytype(input2);
            }

        }
        else if (input1 == ".rename")
        {
            string input2;
            SS>>input2;
            if(input2=="node")
            {
                string oldname,newname;
                SS>>oldname>>newname;
                Circuit. renamenode(oldname,newname);
            }
            else
            {
                cout << "Error: Invalid syntax" << endl;
            }
        }
        else
        {
            cout << "Error: Syntax error" << endl;
        }

    }
    for (Node* node : nodes) {
        delete node;
    }
    nodes.clear();

    for (Component* comp : Components) {
        delete comp;
    }
    Components.clear();


    return 0;
}