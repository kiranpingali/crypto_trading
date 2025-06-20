document.addEventListener('DOMContentLoaded', async () => {
    const pairs = [
        "BTC-USD", "ETH-USD", "USDT-USD", "SOL-USD", "XRP-USD", "DOGE-USD", "ADA-USD", "AVAX-USD", "LINK-USD", "MATIC-USD"
    ];
    const pairSelect = document.getElementById('pair');
    pairs.forEach(pair => {
        const opt = document.createElement('option');
        opt.value = pair;
        opt.textContent = pair;
        pairSelect.appendChild(opt);
    });

    // Fetch and display order book
    const orderbookContainer = document.getElementById('orderbook-container');
    orderbookContainer.innerHTML = '<p>Loading order book...</p>';
    try {
        const res = await fetch('/api/orderbook');
        const data = await res.json();
        orderbookContainer.innerHTML = '';
        Object.keys(data).forEach(pair => {
            const book = data[pair];
            const section = document.createElement('section');
            section.innerHTML = `
                <h3>${pair}</h3>
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
            orderbookContainer.appendChild(section);
        });
    } catch (e) {
        orderbookContainer.innerHTML = `<p class="error">Failed to load order book: ${e.message}</p>`;
    }

    // Handle trade form submission
    const tradeForm = document.getElementById('trade-form');
    const tradeResult = document.getElementById('trade-result');
    tradeForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        tradeResult.innerHTML = '<p>Submitting order...</p>';
        const pair = tradeForm.pair.value;
        const side = tradeForm.side.value;
        const quantity = parseFloat(tradeForm.quantity.value);
        try {
            const res = await fetch('/api/trade', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ pair, side, quantity })
            });
            const data = await res.json();
            if (res.ok) {
                tradeResult.innerHTML = `<pre>${JSON.stringify(data, null, 2)}</pre>`;
            } else {
                tradeResult.innerHTML = `<p class="error">${data.error || 'Trade failed'}</p>`;
            }
        } catch (e) {
            tradeResult.innerHTML = `<p class="error">${e.message}</p>`;
        }
    });
}); 