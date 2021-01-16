(define (problem ijrr_7)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street54 street56 - street
		building7 building10 building18 building28 - building
	)
	(:init
		(atLocation survivor0 street54)
		(atLocation survivor1 building7)
		(onFire building18)
		(onFire building28)
		(underRubble street56)
		(underRubble building10)
		(needsRepair building10)
		(needsRepair building18)
		(needsRepair building28)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street56))
		(not (needsRepair building10))
		(not (needsRepair building18))
		(not (needsRepair building28))
	))
	(:metric minimize total-time)
)
