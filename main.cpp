#include <bits/stdc++.h>
using namespace std;

class Node {
private:
    string name;
    int number;
    double voltage;


public:
    string getName()
    {
        return name;
    }
    double getVoltage()
    {
        return voltage;
    }

    Node(string input, int n) : name(input), number(n), voltage(0.0) {}

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

public:
    Component(string input, Node* n1, Node* n2, string t) : name(input), node1(n1), node2(n2), type(t) {}
    virtual double getVoltage()  = 0;
    virtual double getCurrent()  = 0;

    string getName()
    {
        return name;
    }
    string getType()
    {
        return type;
    }
    Node* getNode1()
    {
        return node1;

    }


    Node* getNode2()  {

        return node2;
    }
};





class Resistor : public Component {
private:
    double resistance;

public:
    Resistor(string input, Node* n1, Node* n2, double r): Component(input, n1, n2, "Resistor"), resistance(r) {}

    double getVoltage()  override
    {
        double v1 = node1->getVoltage();
        double v2 = node2->getVoltage();

        return abs(v1 - v2);
    }

    double getCurrent()  override
    {
        double v1 = node1->getVoltage();
        double v2 = node2->getVoltage();
        double voltage = v1- v2;
        return voltage / resistance;
    }

    double getRes()
    {
        return resistance;
    }

    bool valueError (double res)
    {
        if(res<=0)
            return true;

        else
            return false;
    }

};

class Capacitor : public Component {
private:
    double capacitance;

public:
    Capacitor(string input, Node* n1, Node* n2, double c): Component(input, n1, n2, "Capacitor"), capacitance(c) {}

    double getVoltage()  override
    {
        double v1 = node1->getVoltage();
        double v2 = node2->getVoltage();

        return abs(v1 - v2);
    }

    double getCurrent()  override
    {
        double v1 = node1->getVoltage();
        double v2 = node2->getVoltage();
        double voltage = v1- v2;
        return voltage / capacitance;
    }

    double getCap()
    {
        return capacitance;
    }

    bool valueError (double res)
    {
        if(res<=0)
            return true;

        else
            return false;
    }

};

vector< string> Error;




vector<Node*> nodes;
vector<Component*> Components;
Node* GN;

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
double convertToOhms(const string& value)
{
    regex pattern(R"((\d+(\.\d+)?)(k|M|Ω)?)");
    smatch match;

    if (regex_match(value, match, pattern)) {
        double number = stod(match[1].str());
        string unit = match[3].str();


        if (unit == "k") {
            return number * 1000;
        }

        else if (unit == "M" || unit == "Meg") {
            return number * 1000000;
        }

        else if (unit == "Ω" || unit.empty()) {
            return number;
        }
    }
    else {

        return -1;
    }

    return -1;
}
double convertToFarad(const string& value)
{
    // اصلاح الگوی regex برای پشتیبانی از nF و فاصله بین عدد و واحد
    regex pattern(R"(\s*(\d+(\.\d+)?)(n|u|F|µ)?\s*(F|nF|uF|µF)?)", regex::icase); // پشتیبانی از nF, uF, µF
    smatch match;

    if (regex_match(value, match, pattern)) {
        double number = stod(match[1].str());
        string unit = match[4].str();  // از match[4] برای استخراج واحد استفاده می‌کنیم

        if (unit == "n" || unit == "nF") {
            return number * 0.000000001;  // نانوفاراد
        }
        else if (unit == "u" || unit == "µ" || unit == "uF" || unit == "µF") {
            return number * 0.000001;  // میکروفاراد
        }
        else if (unit == "F" || unit.empty()) {
            return number;  // فاراد
        }
    }
    else {
        return -1;  // ورودی نامعتبر
    }

    return -1;  // ورودی نامعتبر
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
void deleteResistor(string name) {
    auto it = find_if(Components.begin(), Components.end(), [&name](Component* comp) {
        return comp->getName() == name && comp->getType() == "Resistor";
    });

    if (it != Components.end()) {
        delete *it;
        Components.erase(it);
        cout << "Resistor " << name << " deleted successfully." << endl;
    } else {
        cout << "Error: Cannot delete resistor; component not found" << endl;
    }
}

int main()
{
    string line;
    while(getline(cin,line))
    {
        stringstream SS(line);
        string input1;
        SS>>input1;
        if(input1 == "end")
        {
            cout<<"end of app"<<endl;
            break;

        }
        else if(input1 ==  "add")
        {
            string typeName;
            SS>>typeName;
            string type = typeName.substr(0,1);
            if(type == "R")
            {
                string node1, node2, value;
                SS>>node1>>node2>>value;

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
                    cout<<"Error: Resistor "<<typeName<<"already exists in the circuit"<<endl;
                    continue;

                }

                double resistance_in_ohms = convertToOhms(value);

                Resistor* resistor = new Resistor(typeName, n1, n2, resistance_in_ohms);

                if (resistance_in_ohms != -1)
                {
                    cout << "Resistance in ohms: " << resistance_in_ohms << " Ω" << endl;
                }
                else if (resistance_in_ohms == -1)
                {

                    cout << "Error: Syntax error"<< endl;

                }
                if(resistor->valueError(resistance_in_ohms))
                {
                    cout << "Error: Resistance cannot be zero or negative"<< endl;
                }

                Components.push_back(resistor);


            }
            else if(type == "C")
            {
                string node1, node2, value;
                SS>>node1>>node2>>value;

                Node* n1 = findnode(node1);
                Node* n2 = findnode(node2);
                if(findComponent(typeName)!= nullptr)
                {
                    cout<<"Error: Capacitor "<<typeName<<"already exists in the circuit"<<endl;
                    continue;

                }

                double capacitance_in_Farad = convertToFarad(value);

                Capacitor* capacitor = new Capacitor(typeName, n1, n2, capacitance_in_Farad);

                if (capacitance_in_Farad != -1)
                {
                    cout << "Resistance in ohms: " << capacitance_in_Farad << " F" << endl;
                }
                else if (capacitance_in_Farad == -1)
                {

                    cout << "Error: Syntax error"<< endl;

                }
                if(capacitor->valueError(capacitance_in_Farad))
                {
                    cout << "Error: Capacitor cannot be zero or negative"<< endl;
                }

                Components.push_back(capacitor);


            }

        }
        else if (input1 == "delete")
        {
            string typeName;
            SS >> typeName;
            string type = typeName.substr(0, 1);

            if (type == "N")
            {


                deleteNode(typeName);
            }
            else if (type == "R")
            {


                deleteResistor(typeName);
            }
        }
    }

    return 0;
}