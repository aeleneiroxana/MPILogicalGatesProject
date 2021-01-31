#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

using namespace std;

class Gate
{
    protected:
    int Order;
    int* SendTo;
    int Output;
    int Type;
    int NoGates;
    public:
    Gate() {}
    Gate(int order, int* sendTo, int noGates, int type = 0)
    {
        NoGates = noGates;
        Order = order;
        SendTo = sendTo;
        Output = false;
        Type = type;
    }
    int* GetSendTo()
    {
        return SendTo;
    }
    int GetOrder()
    {
        return Order;
    }
    int GetType()
    {
        return Type;
    }
    int GetCount()
    {
        return NoGates;
    }
    int GetOutput()
    {
        return Output;
    }
    virtual void Print()
    {
        cout << "Order: " << Order << " Type: " << Type << " Number of gates to send to: " << NoGates << endl;
        cout << "Gates to send to: ";
        for (int i = 0; i < NoGates; i++)
            cout << SendTo[i] << " ";
    }
    virtual int Execute(int* input)
    {
        Output = input[0]; return Output;
    }
};

class AndGate : public Gate
{
    public:
    AndGate(int order, int* sendTo, int noGates) : Gate(order, sendTo, noGates) {}
    int Execute(int* input)
    {
        bool inp1 = (input[0] == 1);
        bool inp2 = (input[1] == 1);
        Output = (inp1 && inp2);
        return Output;
    }
    void Print()
    {
        cout << "AND GATE - " << "Order: " << Order << " Number of gates to send to: " << NoGates << endl;
        cout << "Gates to send to: ";
        for (int i = 0; i < NoGates; i++)
            cout << SendTo[i] << " ";
    }
};

class NotGate : public Gate
{
    public:
    NotGate(int order, int* sendTo, int noGates) : Gate(order, sendTo, noGates) {}
    int Execute(int* input)
    {
        if (input[0] == 1)
            Output = 0;
        else
            Output = 1;
        return Output;
    }
    void Print()
    {
        cout << "NOT GATE - " << "Order: " << Order << " Number of gates to send to: " << NoGates << endl;
        cout << "Gates to send to: ";
        for (int i = 0; i < NoGates; i++)
            cout << SendTo[i] << " ";
    }
};

class OrGate : public Gate
{
    public:
    OrGate(int order, int* sendTo, int noGates) : Gate(order, sendTo, noGates) {}
    int Execute(int* input)
    {
        bool inp1 = (input[0] == 1);
        bool inp2 = (input[1] == 1);
        Output = (inp1 || inp2);
        return Output;
    }
    void Print()
    {
        cout << "OR GATE - " << "Order: " << Order << " Number of gates to send to: " << NoGates << endl;
        cout << "Gates to send to: ";
        for (int i = 0; i < NoGates; i++)
            cout << SendTo[i] << " ";
    }
};

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        // daca avem rank 0 inseamna ca suntem in procesul initial
        // in care vom citi informatiile dintr-un fisier
        int outputCount = 0; // numaram dimensiunea outputului (in biti)
        map<int, int> outputOrder; // aici tinem minte ordinea outputului, adica perechi de forma (nr. portii, nr. outputului)
        int result[15]; //aici tinem minte rezultatul final
        int n;
        cin >> n; // numarul de elemente din circuit (adica nr. de teste + nr. de porti
        vector<Gate*> gates;
        for (int i = 0; i < n; i++)
        {
            string type;
            cin >> type; // citim tipul de nod, adica test sau poarta ("gate")
            cout << "Read gate type: " << type << endl;
            if (type.compare("test") == 0)
            {
                cout << "We have a test" << endl;
                int input; // daca este de tip test, are o valoare de input, true sau false (1 sau 0)
                cin >> input;
                cout << "Test value: " << input << endl;
                int noGates;
                cin >> noGates; // citim numarul de porti in care va trimite informatia
                cout << "We will read " << noGates << " of gates to send to" << endl;
                int* sendsTo = new int[noGates];
                for (int j = 0; j < noGates; j++)
                    cin >> sendsTo[j]; // citim numerele de ordine ale portilor catre care se va trimite valoarea

                Gate* testGate = new Gate(0, sendsTo, noGates);
                testGate->Execute(new int [] { input });
                testGate->Print();
                gates.push_back(testGate);
            }
            else if (type.compare("gate") == 0)
            {
                cout << "We have a gate" << endl;
                int order, functionType, noGates;// o poarta are un numar de ordine,
                cin >> order >> functionType >> noGates;//un tip (AND[1], OR[2] sau NOT[3]) si un numar de porti
                int* sendsTo = new int[noGates];  //catre care va trimite rezultatul obtinut
                for (int j = 0; j < noGates; j++)
                {
                    cin >> sendsTo[j];
                    if (sendsTo[j] == 0) // daca trimitem catre nodul 0 inseamna ca rezultatul e de tip output
                    {
                        outputCount++; // deci il numaram, ca sa stim sa il asteptam
                        outputOrder.insert(make_pair(order, outputCount)); // si cream perechea ca sa stim ca
                                                                            // de la nodul asta va veni outputul cu numarul asta de ordine
                    }
                }

                Gate* gate = new Gate(order, sendsTo, noGates, functionType);
                gate->Print();
                gates.push_back(gate);
            }
        }

        for (int i = 0; i < n; i++)
        {
            if (gates[i]->GetOrder() == 0) // daca nr. de ordine este 0, inseamna ca avem un nod de tip test
            {
                int output = gates[i]->GetOutput(); // vedem ce valoare are testul
                int count = gates[i]->GetCount(); // catre cate porti trimite
                int* sendsTo = gates[i]->GetSendTo(); // si portile in sine
                for (int j = 0; j < count; j++) // deci luam toate portile catre care trebuie sa trimitem
                {
                    MPI_Send(&output, 1, MPI_INT, sendsTo[j], 4, MPI_COMM_WORLD); // si le trimitem valoarea
                    cout << "We have sent this input: " << output << " to process " << sendsTo[j] << endl;
                }
            }
            else
            { // altfel, deschidem cate un proces pentru fiecare poarta,
                int order = gates[i]->GetOrder(); // ii luam numarul de ordine, ca sa stim catre ce proces trimitem
                int type = gates[i]->GetType(); // tipul de poarta (AND/OR/NOT)
                int* sendsTo = gates[i]->GetSendTo(); // portile catre care trimitem
                int count = gates[i]->GetCount(); // si numarul portilor catre care va trimite
                MPI_Send(&type, 1, MPI_INT, order, 1, MPI_COMM_WORLD); // ii trimitem tipul de poarta
                cout << "We have sent this type: " << type << " to process " << order << endl;
                MPI_Send(&count, 1, MPI_INT, order, 2, MPI_COMM_WORLD); // numarul de porti catre care va trimite
                cout << "We have sent this count: " << count << " to process " << order << endl;
                MPI_Send(sendsTo, count, MPI_INT, order, 3, MPI_COMM_WORLD); // si portile in sine
                cout << "We have sent an array of ints to process " << order << endl;
            }
        }

        for (int i = 1; i <= outputCount; i++) // asteptam perechi de tipul (outputValue, gateNumber)
        {
            int* output; //pentru ca nu stim de la ce porti vom primi rezultate si in ce ordine,
            MPI_Recv(&output, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //acceptam de la oricare
            cout << "We have received this output: " << output[0] << " " << output[1] << endl;
            int outputSlot = outputOrder.find(output[1])->second; //extragem pozitia pe care ar trebui sa fie rezultatul,
            result[outputSlot] = (output[0] == 1); // in functie de ceea ce am stabilit initial
        }

        for (int i = 0; i < outputCount; i++) //afisam rezultatul
            cout << result[i];
    }
    else
    { // in orice al proces, acceptam de la orice sursa ne trimite informatii
        Gate* gate;
        int type, count;
        int* sendsTo;
        int* input;
        MPI_Recv(&type, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//avand taguri pentru fiecare info
        cout << "Process " << rank << " has received this type: " << type << endl;
        MPI_Recv(&count, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Process " << rank << " has received this count: " << count << endl;
        sendsTo = new int[count];
        MPI_Recv(sendsTo, count, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Process " << rank << " has received an array of ints" << endl;
        if (type <= 2) //daca type-ul e mai mic ca 2, e fie AND fie OR, deci accepta 2 inputuri
        {
            input = new int[2];
            int inpVal;
            MPI_Recv(&inpVal, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // pe care le si asteptam
            input[0] = inpVal;
            MPI_Recv(&inpVal, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            input[1] = inpVal;
        }
        else // altfel, asteptam unul singur
        {
            input = new int[1];
            int inpVal;
            MPI_Recv(&inpVal, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            input[0] = inpVal;
        }

        switch (type) // in functie de tip cream poarta de asa fel incat sa proceseze ceea ce ne-am dorit
        {
            case 1:
                gate = new AndGate(rank, sendsTo, count);
                break;
            case 2:
                gate = new OrGate(rank, sendsTo, count);
                break;
            default:
                gate = new NotGate(rank, sendsTo, count);
                break;
        }
        cout << "We are in process " << rank << endl;
        gate->Print();
        int output = gate->Execute(input); //executam operatia respectiva

        for (int i = 0; i < count; i++)
        {
            if (sendsTo[i] == 0) // daca poarta are numarul 0 inseamna ca rezultatul obtinut este de tip output
            {
                int* result = new int[2]; // deci trimitem catre procesul master
                result[1] = rank; // rankul procesului curent
                result[0] = output; // si rezultatul
                MPI_Send(result, 2, MPI_INT, sendsTo[i], 1, MPI_COMM_WORLD);
                cout << "Process " << rank << " has sent the input" << result[0] << " " << result[1] << " to process " << sendsTo[i] << endl;
            }
            else
            { // altfel, trimitem catre urmatorul proces, care deja asteapta, rezultatul procesului curent
                MPI_Send(&output, 1, MPI_INT, sendsTo[i], 4, MPI_COMM_WORLD);
                cout << "Process " << rank << " has sent the input " << output << " to process " << sendsTo[i] << endl;
            }
        }
    }
    MPI_Finalize();
    return 0;
}