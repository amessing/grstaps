(define (problem ijrr_18)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street9 street48 - street
		building20 building27 building28 - building
	)
	(:init
		(atLocation survivor0 street48)
		(atLocation survivor1 building20)
		(onFire building27)
		(onFire building28)
		(underRubble street9)
		(underRubble building28)
		(needsRepair building27)
		(needsRepair building28)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street9))
		(not (needsRepair building27))
		(not (needsRepair building28))
	))
	(:metric minimize total-time)
)
