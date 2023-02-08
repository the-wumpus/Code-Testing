# Thomas Ehret
# CS519 Applied ML
# Dr Cao
# NMSU Sp23
# Homework 1

# necessary libraries
import os
import pandas
import matplotlib.pyplot as plt
import numpy

# purpose: read in the iris.data, extract some information, and visualize extracted data.

# built-in functions: 
# info(), read(), len(), print(), str(),tolist(), unique(),
# value_cpunts(), mean(), max(), min(), where()

# read local data file | change line 20 to your local path for iris.data
# s = 'C:\\Users\\xyzzy\\Desktop\\NMSU courses\\App ML\\mod1\\hw1\\iris.data'
# df = pandas.read_csv(s, header=None, encoding='utf-8')

# check-expect data veracity
df = pandas.read_csv(
    'https://archive.ics.uci.edu/ml/''machine-learning-databases/iris/iris.data', header=None, encoding='utf-8')

# calculate dataset size [rows,columns]
rows = len(df.axes[0])
columns = len(df.axes[1])

# check-expect data frame info
# df.info()

# rename columns for clarity
df.columns = ['Sepal-L', 'Sepal-W', 'Petal-L', 'Petal-W', 'Class']

# Print the number of rows and columns
print("Number of Rows: " + str(rows))
print("Number of Columns: " + str(columns))

# check-expect [50, 4] == Iris-versicolor

# return column data as single element formatted list 
last_column = df[df.columns[-1]].tolist()

# check-expect elements of Class column to list
# print("Elements of last column in dataset" , *last_column, sep= "\n")

# return unique elements column data as list 
print("Unique values of Class column: ",df.Class.unique())

# print(df.value_counts()['Iris-setosa'])
print("Number of Iris-setosa in dataset: ",df['Class'].value_counts()['Iris-setosa'])

# calculate average of first column (sepal length)
print("Mean of Sepal Length (column 1):", df['Sepal-L'].mean())

# calculate max value of second column (sepal width)
print("Max value of Sepal width (column 2):", df['Sepal-W'].max())

# calculate min value of third column (petal length)
print("Minimum value of Petal Length (column 3):", df['Petal-L'].min())

# draw a 3 color and 3 shape scatter plot of sepal length,sepal width (x,y) per Class
# this code adapted from https://github.com/rasbt/machine-learning-book

y = df.iloc[0:150, 4].values
y = numpy.where(y == 'Iris-setosa', 0, 1)

# get column values and select class labels
X = df.iloc[0:150, [0, 1]].values

# plot iris.data 
plt.scatter(X[:50, 0], X[:50,1], color='blue', marker= 's', label="Setosa")
plt.scatter(X[50:100, 0], X[50:100,1], color='red', marker= 'o', label="Versicolor")
plt.scatter(X[100:150, 0], X[100:1509, 1], color='green', marker= 'p', label="Virginica")

# display 3 color 3 shape scatter plot
plt.xlabel('Sepal Length cm')
plt.ylabel('Sepal Width cm')
plt.legend(loc='best')
plt.title("Sepal Properties of Irises")
plt.show()





