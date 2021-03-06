const { ipcRenderer } = require('electron');
class SyntaxTreeNode {
    constructor(id, fa = -1, x = -1, y = -1, offsetx = 0, symbol = "", sonl = 1e9, sonr = -1e9) {
        this.id = id;
        this.fa = fa;
        this.x = x;
        this.y = y;
        this.offsetx = offsetx;
        this.symbol = symbol;
        this.sonl = sonl;
        this.sonr = sonr;
    }
}
;
class SyntaxTree {
    constructor() {
        this.radius = 35; // 绘制出的节点半径 px
    }
    buildFromFile(data) {
        let lines = data.split('\n');
        this.nodes = new Array(lines.length - 2); // 初始化
        for (let i = 0; i < this.nodes.length; i++) {
            this.nodes[i] = new SyntaxTreeNode(i);
            this.nodes[i].son = new Array(); // 初始化
        }
        this.atDepth = new Array();
        for (let i = 0; i < lines.length; i++) { // 处理语法树信息
            if (i == 0) { // 获取根节点标号
                this.root = parseInt(lines[i]);
            }
            else if (i == lines.length - 1) { // 根节点
                this.nodes[this.root].symbol = lines[i].split(' ')[0];
            }
            else { // 解析子节点 当前节点 id = i - 1
                let arr = lines[i].split(' ');
                for (let j = arr.length - 2; j >= 1; j -= 2) {
                    let symbol = arr[j]; // 子节点符号
                    let sonId = parseInt(arr[j - 1]); // 子节点 id
                    if (sonId != -1)
                        this.nodes[i - 1].son.push(sonId), this.nodes[sonId].symbol = symbol;
                    else {
                        this.nodes.length++;
                        this.nodes[this.nodes.length - 1] = new SyntaxTreeNode(this.nodes.length - 1);
                        this.nodes[this.nodes.length - 1].son = new Array(); // 初始化
                        this.nodes[i - 1].son.push(this.nodes.length - 1);
                        this.nodes[this.nodes.length - 1].symbol = symbol;
                    }
                }
            }
        }
        this.initD3Arrow(); // 初始化 d3 箭头
        this.dfs(this.root); // 深搜处理出每一层的节点
        this.setCoordinates(); // 设置坐标
        this.setContainerSize(); // 设置 svg 大小
        this.drawTree(); // 绘制语法树
    }
    setContainerSize() {
        const svg = document.querySelector('#sh_circleContainer');
        svg.style.height = `${svg.getBBox().y + this.atDepth.length * this.radius * 4 + this.radius * 4}`; // 设置 svg 容器高度
        let maxX = 0; // 获取最大 x 最表
        for (let i = 0; i < this.nodes.length; i++)
            maxX = Math.max(maxX, this.nodes[i].x + this.radius * 4);
        svg.style.width = `${svg.getBBox().x + maxX}`;
    }
    dfs(x, depth = 0) {
        let node = this.nodes[x];
        node.y = (depth + 1) * this.radius * 4; // 设置 y 坐标
        if (this.atDepth.length <= depth) {
            this.atDepth.length = depth + 1;
            this.atDepth[depth] = new Array(); // 动态设置大小
        }
        this.atDepth[depth].push(x); // 同一深度的节点放在一起
        for (let i = 0; i < node.son.length; i++) { // 继续遍历子节点
            this.nodes[node.son[i]].fa = x; // 设置节点父亲
            this.dfs(node.son[i], depth + 1);
        }
    }
    setCoordinates() {
        for (let depth = this.atDepth.length - 1; depth >= 0; depth--) { // 预处理每个深度从左到右节点坐标
            let curPos = 0, delta = this.radius * 4;
            for (let i = 0; i < this.atDepth[depth].length; i++) {
                let node = this.nodes[this.atDepth[depth][i]];
                node.x = curPos, curPos += delta;
            }
        } // 可以保证子树不会交叉
        for (let depth = this.atDepth.length - 1; depth >= 0; depth--) { // 从深到浅设置节点实际位置
            for (let i = 0; i < this.atDepth[depth].length; i++) {
                let node = this.nodes[this.atDepth[depth][i]];
                if (node.son.length != 0) { // 叶子节点跳过
                    let postmp = (Math.round(node.son.length) % 2 == 0) ? (this.nodes[node.son[0]].x
                        + this.nodes[node.son[node.son.length - 1]].x) / 2 :
                        this.nodes[node.son[Math.trunc(node.son.length / 2)]].x;
                    let delta = postmp - node.x;
                    node.x = postmp; // 子节点偶数个 设为 son[0], son[length - 1] 的中间; 否则为 length / 2 子节点的位置
                    if (delta < 0) { // 节点相比实际位置左移，左移其左侧节点，右边的节点不影响
                        for (let j = 0; j < i; j++) {
                            let nodej = this.nodes[this.atDepth[depth][j]];
                            nodej.offsetx += delta;
                            this.pushDown(nodej.id, nodej.id); // 下放标记
                        }
                    }
                    else if (delta > 0) { // 相比实际位置右移，右移其右侧节点，左边的节点不影响
                        for (let j = i + 1; j < this.atDepth[depth].length; j++) {
                            let nodej = this.nodes[this.atDepth[depth][j]];
                            nodej.offsetx += delta;
                            this.pushDown(nodej.id, nodej.id); // 下放标记
                        }
                    }
                }
            }
            for (let i = 0; i < this.atDepth[depth].length; i++) {
                let node = this.nodes[this.atDepth[depth][i]];
                this.pushDown(node.id, node.id);
            }
            if (this.atDepth[depth].length != 0) {
                let node = this.nodes[this.atDepth[depth][0]];
                node.offsetx -= node.x;
                this.pushDown(node.id, node.id);
            }
            for (let i = 1; i < this.atDepth[depth].length; i++) {
                let nodel = this.nodes[this.atDepth[depth][i - 1]];
                let node = this.nodes[this.atDepth[depth][i]];
                let shouldBeX = nodel.sonr + node.x - node.sonl + this.radius * 3;
                node.offsetx += shouldBeX - node.x;
                this.pushDown(node.id, node.id);
            }
        }
        //如果有节点 x 坐标过小，对整个语法树进行平移
        let minPos = 1e9;
        for (let i = 0; i < this.nodes.length; i++) {
            minPos = Math.min(minPos, this.nodes[i].x);
            console.log(this.nodes[i].x);
        }
        if (minPos < this.radius * 2)
            this.nodes[this.root].offsetx += this.radius * 2 - minPos;
        this.pushDown(this.root, this.root);
    }
    pushDown(x, root) {
        let node = this.nodes[x];
        node.x += node.offsetx;
        if (node.id == root)
            node.sonl = node.x, node.sonr = node.x;
        else
            this.nodes[root].sonl = Math.min(this.nodes[root].sonl, node.x), this.nodes[root].sonr = Math.max(this.nodes[root].sonr, node.x);
        for (let i = 0; i < node.son.length; i++) {
            this.nodes[node.son[i]].offsetx += node.offsetx;
            this.pushDown(node.son[i], root);
        }
        node.offsetx = 0;
    }
    drawTree() {
        d3.select("#g_circle_and_arrow")
            .selectAll("g")
            .data(this.nodes)
            .join((enter) => {
            // append an element matching the selector and set constant attributes
            const g_enter = enter.append("g");
            g_enter.attr("id", (d, i) => `node${d.id}`);
            g_enter.append("circle")
                .attr("r", this.radius) // 设置半径
                .attr("cx", (d, i) => d.x)
                .attr("cy", (d, i) => d.y)
                .style("fill", "pink")
                .style("fill-opacity", 0.7);
            // append relavtive texts
            g_enter.append("text") // 设置节点内容
                .text((d, i) => d.symbol)
                .attr("x", (d, i) => d.x)
                .attr("y", (d, i) => d.y)
                .classed("circleText", true)
                .style("text-anchor", "middle");
            return g_enter;
        }, // update existing elements
        (update) => { return update; }, (exit) => {
            return exit.remove();
        });
        d3.select("#g_circle_and_arrow")
            .selectAll("line")
            .data(this.nodes)
            .join((enter) => {
            // append an element matching the selector and set constant attributes
            const line_enter = enter.append("line");
            line_enter.attr("id", (d, i) => `node${d.id}`);
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
        }, // update existing elements
        (update) => {
            return update;
        }, (exit) => {
            // exit phase
            return exit.remove();
        });
    }
    initD3Arrow() {
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
        arrow_path = "M10,2 L2,6 L10,10 L6,6 L10,2";
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
};
ipcRenderer.on("Syntax:data", (e, data, dir) => {
    console.log(dir);
    syntaxTree.buildFromFile(data);
});
