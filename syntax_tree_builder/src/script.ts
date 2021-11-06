const { ipcRenderer } = require('electron')

class SyntaxTreeNode {
    constructor(public id: number, public fa: number = -1, public x: number = -1, public y: number = -1,
        public sonXsum = .0, public offsetx: number = 0, public symbol: string = "") { }
    public son: Array<number>
};

class SyntaxTree {
    nodes : Array<SyntaxTreeNode> ;
    atDepth: Array<Array<number>>; // 深度为 depth
    radius: number = 30;
    root: number;
    
    buildFromFile(data): void { 
        let lines = data.split('\n');
        this.nodes = new Array<SyntaxTreeNode>(lines.length - 2);
        for (let i = 0; i < this.nodes.length; i++) {
            this.nodes[i] = new SyntaxTreeNode(i);
            this.nodes[i].son = new Array<number>();
        }
        this.atDepth = new Array<Array<number>>();
        for (let i = 0; i < lines.length; i++) {
            if (i == 0) {
                this.root = parseInt(lines[i]);
            } else if (i == lines.length - 1) {
                this.nodes[this.root].symbol = lines[i].split(' ')[0];
            } else {
                let arr = lines[i].split(' ');
                for (let j = arr.length - 2; j >= 1; j -= 2) {
                    let symbol = arr[j];
                    let sonId = parseInt(arr[j - 1]);
                    if (sonId != -1) this.nodes[i - 1].son.push(sonId), this.nodes[sonId].symbol = symbol;
                    else {
                        this.nodes.length++;
                        this.nodes[this.nodes.length - 1] = new SyntaxTreeNode(this.nodes.length -1);
                        this.nodes[this.nodes.length - 1].son = new Array<number>();

                        this.nodes[i - 1].son.push(this.nodes.length - 1);
                        this.nodes[this.nodes.length - 1].symbol = symbol;
                    }
                }
            }
        }

        // for (let i = 0; i < this.nodes.length; i++) console.log(this.nodes[i].symbol);

        this.initD3Arrow();
        this.dfs(this.root); // 深搜处理出每一层的节点
        this.setCoordinates(); // 设置坐标
        this.setContainerSize(); // 设置 svg 大小
        this.drawTree();
    }

    setContainerSize(): void {
        const svg = (<SVGAElement>document.querySelector('#sh_circleContainer'));
        svg.style.height = `${svg.getBBox().y + this.atDepth.length * this.radius * 4 + this.radius * 4}`; // 设置 svg 容器高度
        let maxX = 0;
        for (let i = 0; i < this.nodes.length; i++) maxX = Math.max(maxX, this.nodes[i].x + this.radius * 4);
        svg.style.width = `${svg.getBBox().x + maxX}`;
    }

    dfs(x: number, depth: number = 0): void {
        let node = this.nodes[x];
        node.y = (depth + 1) * this.radius * 4; // 设置 y 坐标

        if (this.atDepth.length <= depth) {
            this.atDepth.length = depth + 1;
            this.atDepth[depth] = new Array<number>();
        }
        this.atDepth[depth].push(x); // 同一深度的节点放在一起

        for (let i = 0; i < node.son.length; i++) {
            this.nodes[node.son[i]].fa = x;
            this.dfs(node.son[i], depth + 1);
        }
    }

    setCoordinates(): void { // O(n^2)
        for (let depth = this.atDepth.length - 1; depth >= 0; depth--) {
            let curPos = 0, delta = this.radius * 4;
            for (let i = 0; i < this.atDepth[depth].length; i++) {
                let node = this.nodes[this.atDepth[depth][i]];
                node.x = curPos;
                if (node.fa != -1) this.nodes[node.fa].sonXsum += node.x;
                curPos += delta;
            }
        }

        for (let depth = this.atDepth.length - 1; depth >= 0; depth--) {
            for (let i = 0; i < this.atDepth[depth].length; i++) {
                let node = this.nodes[this.atDepth[depth][i]];
                if (node.son.length != 0) {
                    if (node.fa != -1) this.nodes[node.fa].sonXsum -= node.x;
                    
                    let delta = node.sonXsum / node.son.length - node.x;
                    node.x = node.sonXsum / node.son.length; // 坐标设为子节点坐标平均值
                    if (node.fa != -1) this.nodes[node.fa].sonXsum += node.x;
                    
                    if (delta < 0) { // 节点相比实际位置左移，左移其左侧节点，右边的节点不影响
                        for (let j = 0; j < i; j++) {
                            let nodej = this.nodes[this.atDepth[depth][j]];
                            nodej.offsetx += delta;
                            this.pushDown(nodej.id, true); // 下放标记
                        }
                    } else if (delta > 0) { // 相比实际位置右移，右移其右侧节点，左边的节点不影响
                        for (let j = i + 1; j < this.atDepth[depth].length; j++) {
                            let nodej = this.nodes[this.atDepth[depth][j]];
                            nodej.offsetx += delta;
                            this.pushDown(nodej.id, true); // 下放标记
                        }
                    }
                }
            }
        }

        let minPos = 1e9;
        for (let i = 0; i < this.nodes.length; i++) minPos = Math.min(minPos, this.nodes[i].x);
        if (minPos < this.radius * 2) this.nodes[this.root].offsetx += this.radius * 2 - minPos;
        this.pushDown(this.root, true);
    }

    pushDown(x: number, changeFa: boolean): void {
        let node = this.nodes[x];
        if (node.offsetx == 0) return;
        if (changeFa && node.fa != -1) this.nodes[node.fa].sonXsum -= node.x;
        node.x += node.offsetx;
        if (changeFa && node.fa != -1) this.nodes[node.fa].sonXsum += node.x;

        for (let i = 0; i < node.son.length; i++) {
            this.nodes[node.son[i]].offsetx += node.offsetx;
            this.pushDown(node.son[i], true);
        }
        node.offsetx = 0;
    }

    drawTree(): void {
        d3.select("#g_circle_and_arrow")
            .selectAll("g")
            .data(this.nodes)
            .join((enter) => {
                // append an element matching the selector and set constant attributes
                const g_enter = enter.append("g");
                g_enter.attr("id", (d, i) => `node${d.id}`)
                g_enter.append("circle")
                    .attr("r", this.radius) // 设置半径
                    .attr("cx", (d, i) => d.x)
                    .attr("cy", (d, i) => d.y)
                    .style("fill", "pink")
                    .style("fill-opacity", 0.7)
                // append relavtive texts
                g_enter.append("text") // 设置节点内容
                    .text((d, i) => d.symbol)
                    .attr("x", (d, i) => d.x)
                    .attr("y", (d, i) => d.y)
                    .classed("circleText", true)
                    .style("text-anchor", "middle");

                return g_enter;
            },  // update existing elements
            (update) => { return update; },
            (exit) => { // exit phase
                return exit.remove();
            });

        d3.select("#g_circle_and_arrow")
            .selectAll("line")
            .data(this.nodes)
            .join((enter) => {
                // append an element matching the selector and set constant attributes
                const line_enter = enter.append("line");

                line_enter.attr("id", (d, i) => `node${d.id}`)
                line_enter
                    .attr("x1", (d, i) => d.fa == -1 ? 0 : this.nodes[d.fa].x)
                    .attr("y1", (d, i) => d.fa == -1 ? 0 : this.nodes[d.fa].y + this.radius)
                    .attr("x2", (d, i) => d.fa == -1 ? 0 : d.x)
                    .attr("y2", (d, i) => d.fa == -1 ? 0 : d.y - this.radius)
                    .attr("stroke", "red")
                    .attr("stroke-width", 2.5)
                    .attr("marker-end", "url(#arrow)");
                line_enter.attr("id", (d, i) => `arrow${d.id}`);
                return line_enter;
            },  // update existing elements
            (update) => {
                return update;
            },
            (exit) => {
                    // exit phase
                return exit.remove();
            });
    }

    initD3Arrow(): void {
        // arrow
        var svg = d3.select("svg");
        var defs = svg.append("defs");
        var arrowMarker = defs.append("marker")
            .attr("id", "arrow")
            .attr("markerUnits", "strokeWidth")
            .attr("markerWidth", "12")
            .attr("markerHeight", "12")
            .attr("viewBox", "0 0 12 12")
            .attr("refX", "9")
            .attr("refY", "6")
            .attr("orient", "auto");

        var revArrowMarker = defs.append("marker")
            .attr("id", "revArrow")
            .attr("markerUnits", "strokeWidth")
            .attr("markerWidth", "12")
            .attr("markerHeight", "12")
            .attr("viewBox", "0 0 12 12")
            .attr("refX", "3")
            .attr("refY", "6")
            .attr("orient", "auto");

        var arrow_path = "M2,2 L10,6 L2,10 L6,6 L2,2";
        arrowMarker.append("path")
            .attr("d", arrow_path)
            .attr("fill", "red")
            .attr("opacity", 0.7);

        arrow_path = "M10,2 L2,6 L10,10 L6,6 L10,2"
        revArrowMarker.append("path")
            .attr("d", arrow_path)
            .attr("fill", "red")
            .attr("opacity", 0.7);
    }
}

const syntaxTree = new SyntaxTree();

window.onload = (e) => {
    console.log("ready");
    ipcRenderer.send("Syntax:window_ready"); // 窗口加载成功
}

ipcRenderer.on("Syntax:data", (e, data, dir) => {
    console.log(dir);
    syntaxTree.buildFromFile(data);
});