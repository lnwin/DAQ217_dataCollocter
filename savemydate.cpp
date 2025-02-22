#include "savemydate.h"
#include "iostream"
#include <fstream>
#include <QDateTime>
saveMydate::saveMydate(QObject *parent)

{

}
void saveMydate::getData2Save(const std::vector<std::vector<float>>& data)
{
    QString mytime;
    mytime=QDateTime::currentDateTime().toString("yyyy_HH_MM_ss");
    std::ofstream file(filePath.toStdString()+"/"+mytime.toStdString()+".csv");

    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    // Get the number of rows and columns
    int rows = data[0].size();
    int cols = data.size();
    for (int j = 0; j < cols; ++j)
    {
        if( data[j].empty())
        {
            continue;
        }

        file <<"Channel_"+std::to_string(j)+",";
    }
     file << "\n";
    // Write the data to the CSV file
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if(data[j].empty())
            {
                continue;
            }
            file << data[j][i];
            if (j < cols - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
};
