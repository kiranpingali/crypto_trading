document.addEventListener('DOMContentLoaded', async () => {
    const container = document.getElementById('orderbook-container');
    container.innerHTML = '<p>Loading order book...</p>';
    try {
        const res = await fetch('/api/orderbook');
        const data = await res.json();
        container.innerHTML = '';
        Object.keys(data).forEach(pair => {
            const book = data[pair];
            const section = document.createElement('section');
            section.innerHTML = `
                <h2>${pair}</h2>
                <div class="orderbook-table-wrapper">
                    <table class="orderbook-table">
                        <thead><tr><th colspan="2">Bids</th><th colspan="2">Asks</th></tr>
                        <tr><th>Price</th><th>Size</th><th>Price</th><th>Size</th></tr></thead>
                        <tbody>
                            ${Array.from({length: Math.max(book.bids.length, book.asks.length, 10)}).map((_, i) => `
                                <tr>
                                    <td>${book.bids[i] ? book.bids[i][0].toFixed(2) : ''}</td>
                                    <td>${book.bids[i] ? book.bids[i][1].toFixed(6) : ''}</td>
                                    <td>${book.asks[i] ? book.asks[i][0].toFixed(2) : ''}</td>
                                    <td>${book.asks[i] ? book.asks[i][1].toFixed(6) : ''}</td>
                                </tr>
                            `).join('')}
                        </tbody>
                    </table>
                </div>
            `;
            container.appendChild(section);
        });
    } catch (e) {
        container.innerHTML = `<p class="error">Failed to load order book: ${e.message}</p>`;
    }
}); 