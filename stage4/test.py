import os
import subprocess
import networkx as nx
from networkx.drawing.nx_agraph import read_dot
import argparse

def run_cmd(command):
    process = subprocess.run(command, shell=True)
    if process.returncode != 0:
        raise Exception(f"Command failed (return code: {process.returncode})")

def parse_digraph(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    start = content.find("digraph {")
    if start == -1:
        raise ValueError("No digraph found in the file")
    digraph_content = content[start:]
    with open(file_path, 'w') as file:
        file.write(digraph_content)
    return read_dot(file_path)

def normalize_graph(graph):
    label_mapping = {}
    for node, data in graph.nodes(data=True):
        label = data.get('label', node)
        if label in label_mapping:
            graph.nodes[node]['label'] = label_mapping[label]
        else:
            label_mapping[label] = label
            graph.nodes[node]['label'] = label

def are_graphs_equal(graph1, graph2):
    normalize_graph(graph1)
    normalize_graph(graph2)
    return nx.is_isomorphic(graph1, graph2, node_match=lambda x, y: x['label'] == y['label'])

def compare_digraphs(file1, file2):
    graph1 = parse_digraph(file1)
    graph2 = parse_digraph(file2)
    return are_graphs_equal(graph1, graph2)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Pass the name of the test (ex: z08)')
    parser.add_argument('test_name', type=str, help='Test name')
    args = parser.parse_args()

    file1_path = "./saida.dot" 
    file2_basepath = "../tests/E3/"
    file2_path = f"{file2_basepath}{args.test_name}.ref.dot"
    file2_input = f"{file2_basepath}{args.test_name}"

    try:
        run_cmd(f"rm -f input.txt && cp {file2_input} ./input.txt")
        run_cmd(f"make && make dot")
        result = compare_digraphs(file1_path, file2_path)
        if result:
            print("The digraphs are equal.")
        else:
            print("The digraphs are not equal.")
    except Exception as e:
        print(f"An error occurred: {e}")
