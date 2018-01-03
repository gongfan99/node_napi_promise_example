const my_module = require('bindings')('simple-example'),
	assert = require('assert');

(async function(){
	try {
		let a = 3.0,
			b = 7.0,
			c = await my_module.example_method(a, b);
	
		console.log("a = ", a);
		console.log("b = ", b);
		console.log("c = ", c);
		assert.equal(a * b, c);
	
	} catch (err){
		console.log(err.message);
	}
})();