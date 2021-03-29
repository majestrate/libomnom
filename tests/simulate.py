from entity_base import makeNetwork, connect_random, Entity
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.animation as animation


def makeNetGraph(nodes):
    G = nx.Graph()
    for node in nodes:
        G.add_node(node.addr())
        for peer in node.peers():
            edge = node.addr(), peer
            if edge not in G.edges:
                G.add_edge(*edge)
    return G

def makeLabels(G, nodes):
    labels = dict()
    addrs = dict()
    for node in nodes:
        addrs[node.addr()] = node
    for edge in G.nodes:
        labels[edge] = '{} with {}'.format(edge, addrs[edge].getAllIDsKnown())
    return labels


def runsim(numNodes):
    nodes = makeNetwork(numNodes)
    fig, ax = plt.subplots()
    def animate(i):
        step = i % 10
        if step == 1:
            connect_random(nodes)
        if step > 0:
            ent = Entity(i, "test data")
            print('gossiping {}'.format(ent))
            nodes[0].broadcast(ent)

        fig.clear()
        G = makeNetGraph(nodes)
        if len(G.edges) == 0:
            return
        #pos = nx.kamada_kawai_layout(G)
        pos = nx.circular_layout(G)
        nx.draw_networkx_nodes(G, pos, node_size=700)
        nx.draw_networkx_edges(G, pos)
        nx.draw_networkx_labels(G, pos, makeLabels(G, nodes))

    ani = animation.FuncAnimation(fig, animate, interval=500)
    ax = plt.gca()
    ax.margins(0.08)
    plt.axis("off")
    plt.tight_layout()
    plt.show()

    
runsim(50)
